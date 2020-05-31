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
		static LuaDatabase* GetLuaUserData(lua_State* L, int index);
		static void PushLuaUserData(lua_State* L, LuaDatabase* what);
	};

	class LuaDatabase {
	public:
		LuaDatabase(lua_State* L) {
			instance = mysql_init(NULL);
			mysql_options(instance, MYSQL_OPT_NONBLOCK, 0);


			lua_getfield(L, LUA_GLOBALSINDEX, "hook"); // 1
			lua_getfield(L, -1, "Add"); // 2

			lua_pushstring(L, "Tick"); // 3

			LuaUserData<LuaDatabase>::PushLuaUserData(L, this); // 4

			auto LUA = L->luabase;
			LUA->SetState(L);
			LUA->CreateMetaTable(MetaName); // 5
			lua_setmetatable(L, -2); // 4

			lua_pushvalue(L, -1); // 5
			reference = luaL_ref(L, LUA_REGISTRYINDEX); // 4

			lua_pushcfunction(L, Tick); // 5

			lua_call(L, 3, 0); // 1

			lua_pop(L, 1); // 0
		}

	public:
		void Push(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
		}

		int GetSocketStatus();
		bool CheckStatus(LuaDatabase* db) {
			int waiting_state = db->socket_state;
			return waiting_state == 0 || (waiting_state & GetSocketStatus()) != 0;
		}

		void InsertAction(std::shared_ptr<LuaAction> action) {
			queue.push_back(action);
		}

	public:
		static const char *MetaName;
		static int MetaType;

		static LuaDatabase* Get(lua_State* L, int index) {
			return LuaUserData<LuaDatabase>::GetLuaUserData(L, index);
		}

		static int Tick(lua_State* L);

		static const _library library[];

	public:
		MYSQL* instance = nullptr;
		std::deque<std::shared_ptr<LuaAction>> queue;

		std::shared_ptr<LuaAction> current_action = nullptr;

		int reference = LUA_REFNIL;
		int socket_state = 0;
	};
}