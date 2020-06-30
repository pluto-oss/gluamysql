#pragma once
#include "lua.hpp"
#include "GarrysMod/Lua/Interface.h"
#include "mysql.h"
#include <deque>
#include <memory>
#include "gluamysql.h"
#include "luauserdata.h"

namespace gluamysql {
	class LuaAction;
	class LuaDatabase;

	template <>
	class LuaUserData<LuaDatabase> {
	public:
		static LuaDatabase* GetLuaUserData(lua_State* L, int index, bool ignore_null = false);
		static void PushLuaUserData(lua_State* L, LuaDatabase* what);
	};

	extern std::deque<LuaDatabase*> FreedDatabases;

	int GetCurrentDatabaseStatus(MYSQL* instance, int status);
	static int CheckDatabaseStatus(MYSQL* instance, int waiting_state) {
		if (waiting_state == 0) {
			return true;
		}

		int wstate0 = waiting_state & (MYSQL_WAIT_READ | MYSQL_WAIT_WRITE);
		int wstate1 = waiting_state & MYSQL_WAIT_EXCEPT;

		int current_state = GetCurrentDatabaseStatus(instance, waiting_state);
		int cstate0 = current_state & (MYSQL_WAIT_READ | MYSQL_WAIT_WRITE);
		int cstate1 = current_state & MYSQL_WAIT_EXCEPT;

		return wstate0 == cstate0 && wstate0 != 0 || wstate1 == cstate1 && wstate1 != 0;
	}

	class LuaDatabase {
	public:
		static std::deque<LuaDatabase*> open_databases;

		LuaDatabase(lua_State* L) {
			instance = mysql_init(NULL);
			mysql_options(instance, MYSQL_OPT_NONBLOCK, 0);

			// we don't want to actually hold a reference since it can be gced

			lua_newtable(L); // 1 (weak reference table)
			lua_newtable(L); // 2 (metatable)
			lua_pushstring(L, "v"); // 3 (weak reference [v]alues)
			lua_setfield(L, -2, "__mode"); // 2
			lua_setmetatable(L, -2); // 1

			// push the real userdata and add to t.db
			LuaUserData<LuaDatabase>::PushLuaUserData(L, this); // 2

			lua_setfield(L, -2, "db"); // 1
			reference = luaL_ref(L, LUA_REGISTRYINDEX); // 0

			open_databases.push_back(this);
		}

	public:
		void Push(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
			lua_getfield(L, -1, "db");
			lua_remove(L, lua_gettop(L) - 1);
		}

		int GetSocketStatus();
		
		bool CheckStatus() {
			return CheckDatabaseStatus(instance, socket_state);
		}

		void InsertAction(lua_State* L, std::shared_ptr<LuaAction> action) {
			if (queue_reference == LUA_REFNIL || queue_reference == LUA_NOREF) {
				Push(L);
				queue_reference = luaL_ref(L, LUA_REGISTRYINDEX);
			}
			queue.push_back(action);
		}

	public:
		static const char *MetaName;
		static int MetaType;

		static LuaDatabase* Get(lua_State* L, int index, bool ignore_null = false) {
			return LuaUserData<LuaDatabase>::GetLuaUserData(L, index, ignore_null);
		}

		void RunTick(lua_State* L);

		static const _library library[];

	public:
		MYSQL* instance = nullptr;
		std::deque<std::shared_ptr<LuaAction>> queue;

		std::shared_ptr<LuaAction> current_action = nullptr;

		int reference = LUA_REFNIL;
		int queue_reference = LUA_REFNIL;
		int socket_state = 0;
		bool gced = false;
	};
}