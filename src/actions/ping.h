#include "luaactionasync.h"
#include "lua.hpp"
#include "mysql.h"

namespace gluamysql {

	class PingAction : public LuaActionAsync {
	public:
		PingAction(lua_State* L) : LuaActionAsync{ L } {

		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_ping_start(&ret, db->instance);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_ping_cont(&ret, db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (ret != 0) {
				Reject(L, db);
				return;
			}
			PushResolve(L);
			lua_call(L, 0, 0);
		}

	public:
		int ret;
	};

}