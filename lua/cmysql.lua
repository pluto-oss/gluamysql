include "promise.lua"
require "gluamysql"

local env = getfenv(0)

mysql.prepare_cache = setmetatable({}, {__mode = "k", __index = function(self, k)
	local r = {}
	self[k] = r
	return r
end})

local function handle_returns(success, ...)
	if (not success) then
		return nil, ...
	end

	return ...
end

local function handle_resume(co, success, err)
	if (not success) then
		pwarnf("error: %s\n%s", err, debug.traceback(co))
	end
end

local function wait_promise(promise)
	local co = coroutine.running()

	promise
		:next(function(...)
			handle_resume(co, coroutine.resume(co, true, ...))
		end)
		:catch(function(...)
			handle_resume(co, coroutine.resume(co, false, ...))
			print "error?"
			print(promise.trace)
		end)

	return handle_returns(coroutine.yield())
end

-- database library

function env.mysql_init(...)
	return wait_promise(mysql.connect(...))
end

function env.mysql_query(db, query)
	return wait_promise(db:query(query))
end

function env.mysql_autocommit(db, b)
	return wait_promise(db:autocommit(b))
end

function env.mysql_commit(db)
	return wait_promise(db:commit())
end

function env.mysql_rollback(db)
	return wait_promise(db:rollback())
end

-- statement library

function env.mysql_stmt_prepare(db, query)

	local cache = mysql.prepare_cache[db][query]
	if (cache) then
		return cache
	end

	local stmt, err = wait_promise(db:prepare(query))

	if (stmt) then
		mysql.prepare_cache[db][query] = stmt
	end

	return stmt, err
end

function env.mysql_stmt_execute(stmt, ...)
	return wait_promise(stmt:execute(...))
end

function env.mysql_stmt_run(db, query, ...)
	local stmt, err = mysql_stmt_prepare(db, query)
	if (not stmt) then
		return false, err
	end
	return wait_promise(stmt:execute(...))
end

-- entry point

function cmysql(func)
	handle_resume(nil, coroutine.resume(coroutine.create(func)))
end

function env.mysql_cmysql()
	assert(coroutine.running() ~= nil, "not running in coroutine")
end