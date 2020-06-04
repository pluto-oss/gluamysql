#include "luapreparedstatement.h"
#include "actions/preparestatement.h"

using namespace gluamysql;

const char* LuaPreparedStatement::MetaName = "gluamysql::LuaPreparedStatement";
int LuaPreparedStatement::MetaType = 0;

void LuaPreparedStatement::Start(lua_State* L, std::string statement) {
	action = std::make_shared<PrepareStatementAction>(L, this, statement);
	db->InsertAction(action);
}

void LuaPreparedStatement::Push(lua_State* L) {
	action->Push(L);
}

LuaPreparedStatement *LuaUserData<LuaPreparedStatement>::GetLuaUserData(lua_State* L, int index) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	auto ret = LUA->GetUserType<LuaPreparedStatement>(index, LuaPreparedStatement::MetaType);

	if (!ret) {
		lua_pushstring(L, "LuaPreparedStatement is NULL");
		lua_error(L);
	}

	return ret;
}

void LuaUserData<LuaPreparedStatement>::PushLuaUserData(lua_State* L, LuaPreparedStatement* what) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	LUA->PushUserType(what, LuaPreparedStatement::MetaType); // 1
	LUA->CreateMetaTable(LuaPreparedStatement::MetaName);
	lua_setmetatable(L, -2);
}