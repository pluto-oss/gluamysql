#include "luapreparedstatement.h"
#include "actions/executestatement.h"
#include "actions/preparestatement_close.h"

using namespace gluamysql;

static int __tostring(lua_State* L) {
	auto stmt = LuaUserData<LuaPreparedStatement>::GetLuaUserData(L, 1, true);
	if (stmt) {
		lua_pushfstring(L, "%s: %p", LuaPreparedStatement::MetaName, stmt);
	} 
	else {
		lua_pushfstring(L, "%s: NULL", LuaPreparedStatement::MetaName);
	}

	return 1;
}

static int __gc(lua_State* L) {
	auto stmt = LuaUserData<LuaPreparedStatement>::GetLuaUserData(L, 1, true);

	if (stmt && !gluamysql::HasCleanedUpAlready) {
		stmt->db->InsertAction(L, std::make_shared<PrepareStatementCloseAction>(L, stmt));
	}

	return 0;
}

static int parametercount(lua_State* L) {
	auto stmt = LuaUserData<LuaPreparedStatement>::GetLuaUserData(L, 1);

	lua_pushinteger(L, mysql_stmt_param_count(stmt->stmt));
	return 1;
}

static int execute(lua_State* L) {
	auto stmt = LuaUserData<LuaPreparedStatement>::GetLuaUserData(L, 1);

	auto action = std::make_shared<ExecuteStatementAction>(L, 2, stmt);
	action->Push(L);

	stmt->db->InsertAction(L, action);

	return 1;
}

const _library LuaPreparedStatement::library[] = {
	{ "parametercount", parametercount },
	{ "execute", execute },
	{ "__tostring", __tostring },
	{ "__gc", __gc },
	{ 0, 0 }
};