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


			// we don't want to actually hold a reference since it can be gced

			lua_newtable(L); // 4 (add id)
			lua_pushcfunction(L, [](lua_State* L) {
				lua_rawgeti(L, -1, 1);
				lua_getfield(L, -1, "db");
				auto db = LuaUserData<LuaDatabase>::GetLuaUserData(L, -1);
				lua_pushboolean(L, !!db);
				return 1;
			}); // 5
			lua_setfield(L, -2, "IsValid"); // 4

			lua_newtable(L); // 5 (weak reference table)
			lua_newtable(L); // 6 (metatable)
			lua_pushstring(L, "v"); // 7
			lua_setfield(L, -2, "__mode"); // 6
			lua_setmetatable(L, -2); // 5

			// set up the real userdata

			LuaUserData<LuaDatabase>::PushLuaUserData(L, this); // 6

			auto LUA = L->luabase;
			LUA->SetState(L);
			LUA->CreateMetaTable(MetaName); // 7
			lua_setmetatable(L, -2); // 6

			lua_setfield(L, -2, "db"); // 5
			lua_rawseti(L, -2, 1); // 4

			lua_pushvalue(L, -1); // 5
			reference = luaL_ref(L, LUA_REGISTRYINDEX); // 6

			lua_pushcfunction(L, Tick); // 5

			lua_call(L, 3, 0); // 1

			lua_pop(L, 1); // 0
		}

		~LuaDatabase() {
			if (instance) {
				mysql_close(instance);
				instance = nullptr;
			}
		}

	public:
		void Push(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
			lua_rawgeti(L, -1, 1);
			lua_getfield(L, -1, "db");
			lua_remove(L, lua_gettop(L) - 1);
			lua_remove(L, lua_gettop(L) - 1);
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