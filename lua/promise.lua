local function pack(...)
	return {n = select("#", ...), ...}
end
local unpack = function(t)
	return unpack(t, 1, t.n)
end

Promise = not isfunction(Promise) and Promise or {
	__index = PROMISE or {},
}
Promise.version = "1.1.0"

function Promise.new(run, ...)
	local promise = setmetatable({
		func = run,
		args = {n = select("#", ...), ...},
		next = {},
		catch = {},
		trace = debug.traceback(),
		state = "pending"
	}, Promise)

	run(function(...)
		promise:_finish(true, ...)
	end, function(...)
		promise:_finish(false, ...)
	end)

	return promise
end

setmetatable(Promise, {__call = function(self, ...) return self.new(...) end})

local MT = Promise.__index

function MT:Then(fn)
	return self:next(fn)
end

function MT:next(fn)
	self._next = fn
	return self:_try()
end

function MT:Catch(fn)
	return self:catch(fn)
end

function MT:catch(fn)
	self._catch = fn
	return self:_try()
end

function MT:wait()
	local co = coroutine.running()
	if (not co) then
		error("wait must be called from coroutine")
	end

	self:next(function(...)
		coroutine.resume(co, {result = {n = select("#", ...), ...}})
	end):catch(function(err)
		coroutine.resume(co, {error = err})
	end)

	return coroutine.yield()
end

function MT:_try()
	if (self._finished) then
		return self
	end

	local result = self.result
	if (not result) then
		return self
	end

	if (result.success and self._next) then
		self._finished = true
		self._next(unpack(result.data))
	elseif (not result.success and self._catch) then
		self._finished = true
		self._catch(unpack(result.data))
	end

	return self
end

function MT:_finish(success, ...)
	if (self.result) then
		return
	end

	self.result = {
		success = success,
		data = pack(...)
	}

	return self:_try()
end
