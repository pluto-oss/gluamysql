#include "luapreparedstatement.h"
#include "actions/executestatement.h"

using namespace gluamysql;

static int __tostring(lua_State* L) {
	auto stmt = LuaUserData<LuaPreparedStatement>::GetLuaUserData(L, 1);
	if (!stmt) {
		lua_pushfstring(L, "[NULL] %s", LuaPreparedStatement::MetaName);
	}
	else {
		lua_pushfstring(L, "%s: %p", LuaPreparedStatement::MetaName, stmt);
	}

	return 1;
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

	stmt->db->InsertAction(action);

	return 1;
}

const _library LuaPreparedStatement::library[] = {
	{ "parametercount", parametercount },
	{ "execute", execute },
	{ "__tostring", __tostring },
	{ 0, 0 }
};