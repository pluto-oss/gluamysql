#include "luadatabase.h"
#include "actions/query.h"
#include "lua.hpp"

using namespace gluamysql;

const char *LuaDatabase::MetaName = "gluamysql::LuaDatabase";


static int IsValid(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);
	lua_pushboolean(L, !!db);
	return 1;
}

static int __gc(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);
	if (!db) {
		return 0;
	}

	delete db;

	*(LuaDatabase * *)lua_touserdata(L, 1) = nullptr;

	return 0;
}

static int __tostring(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);
	if (!db) {
		lua_pushfstring(L, "[NULL] %s", LuaDatabase::MetaName);
	}
	else {
		lua_pushfstring(L, "%s: %p", LuaDatabase::MetaName, db);
	}

	return 1;
}

static int query(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	if (!db)
		luaL_typerror(L, 1, LuaDatabase::MetaName);

	if (!lua_isstring(L, 2))
		luaL_typerror(L, 2, "string");


	size_t size;
	const char* c_str = lua_tolstring(L, 2, &size);
	std::string str(c_str, size);

	auto promise = std::make_shared<gluamysql::QueryAction>(L, str);
	db->InsertAction(promise);

	promise->Push(L);

	return 1;
}

const _library LuaDatabase::library[] = {
	{ "__gc", __gc },
	{ "__tostring", __tostring },
	{ "IsValid", IsValid },
	{ "query", query },
	{ 0, 0 }
};

int LuaDatabase::MetaType;