include "promise.lua"
require "gluamysql"
local config = {
	host = "localhost",
	username = "username",
	password = "password",
	database = "test",
}

mysql.connect(
	config.host,
	config.username,
	config.password,
	config.database
)
	:next(function(db)
		print("CONNECTED TO DATABASE: ", db)
		db:query("SELECT 1 as number")
			:next(function(data)
				print "SELECTED DATA:"
				PrintTable(data)
			end)
			:catch(function(err)
				print("ERROR SELECTING 1: " .. err)
			end)
			
		db:prepare("SELECT ? as data")
			:next(function(stmt)
				print("PREPARED: ", stmt)
				print("PARAMETER COUNT", stmt:parametercount())

				stmt:execute "76561198050165746"
					:next(function(t)
						print("SELECTED 76561198050165746: ", t[1].data)
						PrintTable(t)
					end)
					:catch(function(err)
						print("FAILED SELECTING 76561198050165746: " .. err)
					end)

				stmt:execute "4"
					:next(function(t)
						print("SELECTED 4: ", t[1].data)
						PrintTable(t)
					end)
					:catch(function(err)
						print("FAILED SELECTING 4: " .. err)
					end)

			end)
			:catch(function(err)
				print("PREPARE ERR", err)
			end)
	end)
	:catch(function(err)
		print("CONNECTION ERROR" .. err)
	end)