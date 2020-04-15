#include "GarrysMod/Lua/Interface.h"
#include "gmodmysql.h"
#include "mysql.h"
#include "mysqldatabase.h"

int gmodmysql::UserDatas::Database = 0;

static void populate_table(GarrysMod::Lua::ILuaBase *LUA, gmodmysql::library lib) {
	for (; lib->name != nullptr; lib++) {
		LUA->PushCFunction(lib->fn);
		LUA->SetField(-2, lib->name);
	}
}

LUA_FUNCTION(DatabaseThunk) {
	auto it = gmodmysql::MySQLDatabase::action_map.cbegin();
	while (it != gmodmysql::MySQLDatabase::action_map.cend()) {
		auto &db = it->first;
		auto actions = it->second;

		while (actions.size() > 0) {
			auto& action = actions[0];
			if (action->TryFinish(LUA)) {
				actions.pop_front();
				delete action;
			}
			else
				break;
		}

		if (actions.size() == 0) {
			it = gmodmysql::MySQLDatabase::action_map.erase(it);
		}
	}

	return 0;
}

GMOD_MODULE_OPEN() {
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->CreateTable();
	populate_table(LUA, gmodmysql::mysqllibrary);
	LUA->SetField(-2, "mysql");
	LUA->Pop(1);

	gmodmysql::UserDatas::Database = LUA->CreateMetaTable("MySQLDatabase");
	populate_table(LUA, gmodmysql::mysqldatabase);
	LUA->Push(-1);
	LUA->SetField(-2, "__index");
	LUA->Pop(1);

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Add");
	LUA->PushString("Tick");
	LUA->PushString("gmodmysql::Thunk");
	LUA->PushCFunction(DatabaseThunk);
	LUA->Call(3, 0);

	return 0;
}

GMOD_MODULE_CLOSE() {
	return 0;
}