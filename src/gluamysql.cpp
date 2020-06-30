#include "gluamysql.h"
#include "luadatabase.h"
#include "luaactionasync.h"
#include "actions/connect.h"

using namespace gluamysql;

static int mysql_connect(lua_State* L) {
	auto db = new LuaDatabase(L); // +1

	if (!lua_isstring(L, 1)) {
		luaL_typerror(L, 1, "string");
	}
	if (!lua_isstring(L, 2)) {
		luaL_typerror(L, 2, "string");
	}
	if (!lua_isstring(L, 3)) {
		luaL_typerror(L, 3, "string");
	}
	if (!lua_isstring(L, 4)) {
		luaL_typerror(L, 4, "string");
	}

	auto query = std::make_shared<gluamysql::ConnectAction>(L, lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tostring(L, 4), lua_isnumber(L, 5) ? static_cast<unsigned int>(lua_tonumber(L, 5)) : 3306);

	db->InsertAction(L, query);
	lua_pop(L, 1);
	query->Push(L);

	return 1;
}

const _library gluamysql::library[] = {
	{ "connect", mysql_connect },
	{ 0, 0 }
};