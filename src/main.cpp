#include "GarrysMod/Lua/Interface.h"
#include "gluamysql.h"
#include "mysql.h"
#include "mysqldatabase.h"
#include "mysqlquery.h"

int gluamysql::UserDatas::MySQLDatabase = 0;

static void populate_table(GarrysMod::Lua::ILuaBase *LUA, gluamysql::library lib) {
	for (; lib->name != nullptr; lib++) {
		LUA->PushCFunction(lib->fn);
		LUA->SetField(-2, lib->name);
	}
}

LUA_FUNCTION(DatabaseThunk) {
	for (
		auto it = gluamysql::MySQLDatabase::action_map.begin();
		it != gluamysql::MySQLDatabase::action_map.end();
		it++
	) {
		auto &db = it->first;
		auto reference = it->second.reference;
		auto &actions = it->second.actions;

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
			it = gluamysql::MySQLDatabase::action_map.erase(it);
			if (reference != 0)
				LUA->ReferenceFree(reference);
		}
	}

	return 0;
}

#define CREATE_METATABLE(name) \
	gluamysql::UserDatas:: name = LUA->CreateMetaTable(#name);\
	populate_table(LUA, gluamysql:: name ::Library);\
	LUA->Pop(1)

GMOD_MODULE_OPEN() {
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->CreateTable();
	populate_table(LUA, gluamysql::mysqllibrary);
	LUA->SetField(-2, "mysql");
	LUA->Pop(1);

	CREATE_METATABLE(MySQLDatabase);
	CREATE_METATABLE(MySQLQuery);

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Add");
	LUA->PushString("Tick");
	LUA->PushString("gluamysql::Thunk");
	LUA->PushCFunction(DatabaseThunk);
	LUA->Call(3, 0);

	return 0;
}

GMOD_MODULE_CLOSE() {
	return 0;
}