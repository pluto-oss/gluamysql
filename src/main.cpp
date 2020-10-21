#include "GarrysMod/Lua/Interface.h"
#include "gluamysql.h"
#include "lua.hpp"
#include "luapromise.h"
#include "luadatabase.h"
#include "luapreparedstatement.h"
#include <cstring>

#define SHUTDOWN_LOADER "return function(override)\n\
	local function f()\n\
		local gm = gmod.GetGamemode() or GM or GAMEMODE\n\
		local old_fn = gm.ShutDown\n\
		gm.ShutDown = function()\n\
			if (old_fn) then\n\
				old_fn()\n\
			end\n\
			override()\n\
		end\n\
	end\n\
	\n\
	if (gmod.GetGamemode()) then\n\
		f()\n\
	else\n\
		hook.Add(\"Initialize\", \"gluamysql::InitializeShutDownHook\", f)\n\
	end\n\
end"

static bool TickHookTick(lua_State* L) {
	bool unready = false;

	auto& list = gluamysql::LuaDatabase::open_databases;

	for (auto it = list.begin(); it != list.end(); ) {
		auto db = *it;
		if (db->current_action != nullptr || db->queue.size() > 0) {
			db->RunTick(L);
			unready = true;
		}
		else if (db->gced) {
			// NOTE: does not work on my vs2019 debug build????
			it = list.erase(it);

			delete db;
			continue;
		}
		it++;
	}

	return !unready;
}

static int TickHook(lua_State * L) {
	TickHookTick(L);

	return 0;
}

DLL_EXPORT int gmod13_open(lua_State* L) {
	gluamysql::PushLibrary(L, gluamysql::library);
	lua_setfield(L, LUA_GLOBALSINDEX, "mysql");

	gluamysql::CreateMetaTable(L, gluamysql::LuaDatabase::MetaName, gluamysql::LuaDatabase::library);
	gluamysql::CreateMetaTable(L, gluamysql::LuaPreparedStatement::MetaName, gluamysql::LuaPreparedStatement::library);

	lua_getglobal(L, "hook");
	lua_getfield(L, -1, "Add");
	lua_pushstring(L, "Tick");
	lua_pushstring(L, "gluamysql::MiscQueueTick");
	lua_pushcfunction(L, TickHook);
	lua_call(L, 3, 0);
	lua_pop(L, 1);

	// this is a hack to be the last thing ShutDown calls since gmod13_close doesn't work how we want (it's called after other __gc things)

	if (luaL_loadbuffer(L, SHUTDOWN_LOADER, strlen(SHUTDOWN_LOADER), "=gluamysql::ShutDownLoader") == 0) {
		lua_call(L, 0, 1);
		lua_pushcfunction(L, [](lua_State* L) {
			printf("gluamysql::ShutDownHook: finishing remaining queries...\n");
			while (!TickHookTick(L)) {
			}

			printf("gluamysql::ShutDownHook: finished!\n");

			return 0;
		});

		lua_call(L, 1, 0);
	}
	else {
		lua_error(L);
	}

	return 0;
}


DLL_EXPORT int gmod13_close(lua_State* L) {
	return 0;
}