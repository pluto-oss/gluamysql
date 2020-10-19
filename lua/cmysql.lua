--[[
	SEE https://github.com/pluto-oss/gluamysql/blob/master/lua/cmysql_example.lua IF YOU WANT TO HAVE AN EXAMPLE OF HOW TO USE THIS FILE

]]

include "promise.lua"
require "gluamysql"

local env = setmetatable({}, {__index = getfenv(0)})

local function handle_returns(success, ...)
	if (not success) then
		error(...)
	end

	return ...
end

local function handle_resume(success, err)
	if (not success) then
		print("ERROR IN RESUME", err)
	end
end

local function wait_promise(promise)
	local co = coroutine.running()

	promise
		:next(function(...)
			handle_resume(coroutine.resume(co, true, ...))
		end)
		:catch(function(...)
			handle_resume(coroutine.resume(co, false, ...))
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
	return wait_promise(db:prepare(query))
end

function env.mysql_stmt_execute(stmt, ...)
	return wait_promise(stmt:execute(...))
end

function env.mysql_stmt_run(db, query, ...)
	return wait_promise(mysql_stmt_prepare(db, query):execute(...))
end

-- entry point

function cmysql(func)
	setfenv(func, env)
	handle_resume(coroutine.resume(coroutine.create(func)))
end