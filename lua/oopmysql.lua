mysql = mysql or {}

local CONNECTION = {}
local Connection = { __index = CONNECTION }
function Connection.new(db)
	local self = setmetatable({}, Connection)
	self.db = db
	return self
end

function CONNECTION:_wait_promise(promise)
	local result = promise:wait()
	if (result.error) then
		error(result.error)
	end
	return result.result
end

function CONNECTION:ping()
	return self:_wait_promise(self.db:ping())
end

function CONNECTION:query(query, ...)
	return self:_wait_promise(self.db:query(query, ...))
end

function CONNECTION:autocommit(autocommit)
	return self:_wait_promise(self.db:autocommit(autocommit))
end

function CONNECTION:commit()
	return self:_wait_promise(self.db:commit())
end

function CONNECTION:rollback()
	return self:_wait_promise(self.db:rollback())
end

function CONNECTION:prepare(query)
	return self:_wait_promise(self.db:prepare(query))
end


local POOL = {}
mysql.pool = mysql.pool or {__index = POOL}
mysql.pool.version = "1.0.0"
setmetatable(mysql.pool, {__call = function(self, ...) return self.new(...) end})

function mysql.pool.new(host, user, password, database, port, max_connections)
	local pool = setmetatable({
		errors = {},
		waiting_threads = {},
		active_threads = {} -- [co] = connection
	}, mysql.pool)

	for i = 1, max_connections do
		mysql.connect(host, user, password, database, port):next(function(conn)
			pool:ReturnConnection(conn)
			table.insert(pool, conn)
		end):catch(function(err)
			table.insert(pool.errors, err)
			ErrorNoHalt(err)
		end)
	end
end

--[[
	Fetches a connection from a pool and creates a coroutine using the function provided.
	If there are no connections available, it will wait until one is available.
]]
function POOL:RequestConnection(func)
	local db, self[#pool] = self[#pool], nil

	if (db) then
		self:CreateThread(func, db)
	else
		table.insert(self.waiting_threads, func)
	end

	self:CheckThreadsForErrors()

	return self
end


--[[
	Checks all active threads for errors and revives the connections.
]]
function POOL:CheckThreadsForErrors()
	for co, conn in pairs(self.active_threads) do
		if (coroutine.status(co) == "dead") then
			self:ReturnConnection(conn)
			self.active_threads[co] = nil
			self:RequestConnection()
			conn.reject()
		end
	end
end

--[[
	Creates a coroutine using the function provided and passes the connection to it.
	Returns a promise that will be resolved when the coroutine finishes.
]]
function POOL:CreateThread(func, db)
	local conn = setmetatable({db = db}, Connection)

	local co = coroutine.create(function(conn)
		conn.promise = Promise(function(resolve, reject)
			conn.reject = reject
			conn:autocommit(true)
			resolve(func(db))
		end)
	end)
	self.active_threads[co] = conn

	coroutine.resume(co, conn)

	conn.promise:next(function()
		self.active_threads[co] = nil
		self:ReturnConnection(conn)
	end)

	return conn.promise
end

--[[
	Returns a connection to the pool, or executes any waiting threads if the pool is waiting.
]]
function POOL:ReturnConnection(conn)
	local inst = conn.db
	if (not inst) then
		return self
	end

	-- disables 
	conn.db = nil

	local waiting, self.waiting_threads[#self.waiting_threads] = self.waiting_threads[#self.waiting_threads], nil
	if (waiting) then
		self:CreateThread(waiting, inst)
	else
		table.insert(self, inst)
	end

	return self
end