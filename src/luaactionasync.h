#pragma once
#include "luaaction.h"
#include "luadatabase.h"

namespace gluamysql {
	class LuaActionAsync : public LuaAction {
	public:
		LuaActionAsync(lua_State* L) : LuaAction{ L } { }

		virtual int Start(lua_State* L, LuaDatabase* db) = 0;
		virtual int Continue(lua_State* L, LuaDatabase* db) = 0;

		bool Query(lua_State* L, LuaDatabase* db) override {
			if (!started) {
				started = true;
				db->socket_state = db->GetSocketStatus();
				db->socket_state = Start(L, db);
			}
			else if (db->CheckStatus()) {
				db->socket_state = db->GetSocketStatus();
				db->socket_state = Continue(L, db);
			}
			
			return db->socket_state == 0;
		}

	private:
		bool started = false;
	};
}