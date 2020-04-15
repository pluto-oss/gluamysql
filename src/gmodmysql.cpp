#include "gmodmysql.h"
#include "mysqldatabase.h"
#include "mysql.h"
#include "GarrysMod/Lua/Interface.h"

#include "mysqlconnect.h"

LUA_FUNCTION(mysql__connect) {
	gmodmysql::MySQLDatabase::Instance database = std::make_shared<gmodmysql::MySQLDatabase::Instance::element_type>();
	
	mysql_init(database.get());
	mysql_options(database.get(), MYSQL_OPT_NONBLOCK, 0);

	database.InsertAction(new gmodmysql::MySQLConnect(database, LUA->CheckString(1), LUA->CheckString(2), LUA->CheckString(3), LUA->GetString(4), LUA->IsType(5, GarrysMod::Lua::Type::Number) ? static_cast<unsigned int>(LUA->CheckNumber(5)) : 0, nullptr, 0));

	gmodmysql::MySQLDatabase::PushUserData(LUA, database);

	return 1;
}

gmodmysql::library gmodmysql::mysqllibrary = {
	{ "connect", mysql__connect },
	{nullptr, nullptr}
};