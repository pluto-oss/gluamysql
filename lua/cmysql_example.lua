include "cmysql.lua"

local config = {
	host = "localhost",
	username = "username",
	password = "password",
	database = "test",
}

cmysql(function()
	local db = mysql_init(config.host, config.username, config.password, config.database)
	print("CMYSQL DB", db)

	-- test regular query

	assert(1 == mysql_query(db, "SELECT 1 as number")[1].number)
	assert(2 == mysql_query(db, "SELECT 2 as number")[1].number)

	print "REGULAR QUERY TEST PASS"

	local stmt = mysql_stmt_prepare(db, "SELECT ? as number")
	print("STATEMENT", stmt)

	assert(3 == mysql_stmt_execute(stmt, 3)[1].number)
	assert(4 == mysql_stmt_execute(stmt, 4)[1].number)

	print "REGULAR STATEMENT TEST PASS"

	mysql_query(db, "CREATE TABLE IF NOT EXISTS t (stuff int)")
	mysql_autocommit(db, false)

	PrintTable(mysql_query(db, "INSERT INTO t (stuff) VALUES (1)"))
	PrintTable(mysql_query(db, "SELECT * from t"))

	mysql_rollback(db)
	mysql_autocommit(db, true)

	print "COMMITTED"
end)