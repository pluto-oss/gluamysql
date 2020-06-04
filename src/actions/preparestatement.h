#pragma once

#include "lua.hpp"
#include "mysql.h"
#include "luapreparedstatement.h"
#include "luaactionasync.h"
#include <string>

namespace gluamysql {
	class PrepareStatementAction : public LuaActionAsync {
	public:
		PrepareStatementAction(lua_State* L, LuaPreparedStatement* _stmt, std::string _statement) : LuaActionAsync{ L }, statement(_statement), stmt(_stmt) {

		}

		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_prepare_start(&out, stmt->stmt, statement.c_str(), statement.length());
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_prepare_cont(&out, stmt->stmt, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (out != 0) {
				Reject(L, db);
			}
			else {
				PushResolve(L); // 1

				// we don't want to actually hold a reference since it can be gced

				lua_newtable(L); // 2 (weak reference table)
				lua_newtable(L); // 3 (metatable)
				lua_pushstring(L, "v"); // 4
				lua_setfield(L, -2, "__mode"); // 3
				lua_setmetatable(L, -2); // 2

				// set up the real userdata

				LuaUserData<LuaPreparedStatement>::PushLuaUserData(L, stmt); // 3

				lua_setfield(L, -2, "prepared"); // 2

				lua_pushvalue(L, -1); // 3

				stmt->reference = luaL_ref(L, LUA_REGISTRYINDEX); // 2
				lua_getfield(L, -1, "prepared"); // 3
				lua_remove(L, lua_gettop(L) - 1); // 2

				lua_call(L, 1, 0);
			}
		}

	public:
		int out = 0;
		std::string statement;
		LuaPreparedStatement* stmt;
	};
}