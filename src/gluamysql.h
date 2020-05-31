#pragma once
#include "lua.hpp"
#include "GarrysMod/Lua/Interface.h"

namespace gluamysql {
	struct _library {
		const char *name;
		lua_CFunction func;
	};

	extern const _library library[];

	static void PushLibrary(lua_State* L, const _library* lib) {
		lua_newtable(L);
		while (lib->name) {
			lua_pushcfunction(L, lib->func);
			lua_setfield(L, -2, lib->name);

			lib++;
		}
	}

	static int CreateMetaTable(lua_State* L, const char* name, const _library* lib) {
		auto LUA = L->luabase;
		LUA->SetState(L);

		int ret = LUA->CreateMetaTable(name);

		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		while (lib->name) {
			lua_pushcfunction(L, lib->func);
			lua_setfield(L, -2, lib->name);

			lib++;
		}

		lua_pop(L, 2);

		return ret;
	}

	static int PushUserdataInfo(lua_State* L) {
		lua_getfield(L, LUA_REGISTRYINDEX, "UserdataInfo");
	}

	template <typename T>
	static void PushUserdata(T *what, void (*push)(lua_State*, T what)) {

	}

	int create(lua_State* L);
}
