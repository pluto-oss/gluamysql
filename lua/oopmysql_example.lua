include "promise.lua"
include "oopmysql.lua"

local config = {
	host = "localhost",
	username = "username",
	password = "password",
	database = "test",
	port = 3306,
	max_connections = 10,
}

our_pool = mysql.pool(config.host, config.user, config.password, config.database, config.port, config.max_connections)

our_pool:RequestConnection(function(conn)
	print("Connection", conn)
	local data = conn:query("SELECT 1 as number")
	print("Data", data[1].number)

	local stmt = conn:prepare("SELECT ? as number")

	for i = 1, 10 do
		local result = stmt:execute(i):wait()
		if (result.error) then
			error(result.error)
		end

		print("Result", result.result[1].number)
	end
end)