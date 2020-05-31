local function pack(...)
	return {n = select("#", ...), ...}
end
local unpack = function(t)
	return unpack(t, 1, t.n)
end

PROMISE = PROMISE or {
	__index = {}
}

local MT = PROMISE.__index

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

function Promise(run)
	local promise = setmetatable({}, PROMISE)
	run(function(...)
		promise:_finish(true, ...)
	end, function(...)
		promise:_finish(false, ...)
	end)

	return promise
end