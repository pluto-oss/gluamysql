#pragma once

#include "luaactionasync.h"
#include "lua.hpp"
#include "mysql.h"

namespace gluamysql {
	class CloseAction : public LuaActionAsync {
	public:
		CloseAction(lua_State* L) : LuaActionAsync{ L } {
		}
		
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_close_start(db->instance);
		}

		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_close_cont(db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
		}
	};
}