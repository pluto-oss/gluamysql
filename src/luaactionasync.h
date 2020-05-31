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
			int state_needed = db->socket_state;
			db->socket_state = db->GetSocketStatus();
			if (!started) {
				started = true;
				db->socket_state = Start(L, db);
			}
			else if ((state_needed & db->socket_state) != 0) {
				db->socket_state = Continue(L, db);
			}
			
			return IsDone(db);
		}

		bool IsDone(LuaDatabase *db) {
			return db->socket_state == 0;
		}

	private:
		bool started = false;
	};
}