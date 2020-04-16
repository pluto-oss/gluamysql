#include "gluamysql.h"
#include "mysqldatabase.h"
#include "mysql.h"
#include "GarrysMod/Lua/Interface.h"

#include <thread>

#include "mysqlconnect.h"

static void Destructor(MYSQL *db) {
	auto thread = std::thread([db]() {
		mysql_close(db);
		delete db;
	});
	thread.detach();
}

LUA_FUNCTION(mysql__connect) {
	gluamysql::MySQLDatabase::Instance database(mysql_init(nullptr), Destructor);
	mysql_options(database.get(), MYSQL_OPT_NONBLOCK, 0);

	database.InsertAction(LUA, new gluamysql::MySQLConnect(database, LUA->CheckString(1), LUA->CheckString(2), LUA->CheckString(3), LUA->GetString(4), LUA->IsType(5, GarrysMod::Lua::Type::Number) ? static_cast<unsigned int>(LUA->CheckNumber(5)) : 0, nullptr, 0));

	gluamysql::MySQLDatabase::PushUserData(LUA, &database);

	return 1;
}

gluamysql::library gluamysql::mysqllibrary = {
	{ "connect", mysql__connect },
	{nullptr, nullptr}
};