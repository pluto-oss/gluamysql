#include "GarrysMod/Lua/Interface.h"
#include "gluamysql.h"
#include "lua.hpp"
#include "luapromise.h"
#include "luadatabase.h"
#include "luapreparedstatement.h"


DLL_EXPORT int gmod13_open(lua_State *L) {
	gluamysql::PushLibrary(L, gluamysql::library);
	lua_setfield(L, LUA_GLOBALSINDEX, "mysql");

	gluamysql::CreateMetaTable(L, gluamysql::LuaDatabase::MetaName, gluamysql::LuaDatabase::library);
	gluamysql::CreateMetaTable(L, gluamysql::LuaPreparedStatement::MetaName, gluamysql::LuaPreparedStatement::library);

	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "gluamysql::UserdataInfo");

	return 0;
}


GMOD_MODULE_CLOSE() {

	return 0;

}