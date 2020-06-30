#include "luadatabase.h"
#include "actions/query.h"
#include "lua.hpp"
#include "luapreparedstatement.h"
#include "actions/autocommit.h"
#include "actions/close.h"

using namespace gluamysql;

const char *LuaDatabase::MetaName = "gluamysql::LuaDatabase";


static int IsValid(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);
	lua_pushboolean(L, !!db);
	return 1;
}

static int __gc(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1, true);

	if (!db) {
		return 0;
	}

	// TODO(meep): clear the things in the tick hook first

	luaL_unref(L, LUA_REGISTRYINDEX, db->reference);
	db->reference = LUA_NOREF;

	db->gced = true;

	db->queue.push_back(std::make_shared<CloseAction>(L));

	ClearUserData(L, 1);

	return 0;
}

static int __tostring(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1, true);

	if (db) {
		lua_pushfstring(L, "%s: %p", LuaDatabase::MetaName, db);
	}
	else {
		lua_pushfstring(L, "%s: NULL", LuaDatabase::MetaName);
	}

	return 1;
}

// direct bindings

static int query(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	if (!lua_isstring(L, 2))
		luaL_typerror(L, 2, "string");


	size_t size;
	const char* c_str = lua_tolstring(L, 2, &size);
	std::string str(c_str, size);

	auto promise = std::make_shared<QueryAction>(L, str);
	db->InsertAction(L, promise);

	promise->Push(L);

	return 1;
}

static int prepare(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	if (!lua_isstring(L, 2))
		luaL_typerror(L, 2, "string");


	size_t size;
	const char* c_str = lua_tolstring(L, 2, &size);
	std::string str(c_str, size);

	auto statement = new LuaPreparedStatement(L, db, str);
	statement->Push(L);

	return 1;
}

static int autocommit(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	if (!lua_isboolean(L, 2))
		luaL_typerror(L, 2, "boolean");

	auto promise = std::make_shared<AutoCommitAction>(L, lua_toboolean(L, 2));
	db->InsertAction(L, promise);

	promise->Push(L);

	return 1;
}

static int commit(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	auto promise = std::make_shared<CommitAction>(L);
	db->InsertAction(L, promise);

	promise->Push(L);

	return 1;
}

static int rollback(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	auto promise = std::make_shared<RollbackAction>(L);
	db->InsertAction(L, promise);

	promise->Push(L);

	return 1;
}

// implementation api

static int queuelength(lua_State* L) {
	auto db = LuaDatabase::Get(L, 1);

	lua_pushnumber(L, db->current_action == nullptr ? 0 : 1 + db->queue.size());

	return 1;
}

const _library LuaDatabase::library[] = {
	{ "__gc", __gc },
	{ "__tostring", __tostring },

	// direct bindings to mysql_ c api
	{ "query", query },
	{ "prepare", prepare },
	{ "autocommit", autocommit },
	{ "commit", commit },
	{ "rollback", rollback },

	// internal implementations 
	{ "IsValid", IsValid },
	{ "queuelength", queuelength },

	{ 0, 0 }
};

int LuaDatabase::MetaType;