#pragma once

#include "GarrysMod/Lua/LuaBase.h"
#include <forward_list>
#include "action.h"
#include "mysql.h"


namespace gluamysql {
	typedef struct function {
		const char *name;
		GarrysMod::Lua::CFunc fn;
	} function;
	using library = function[];

	extern library mysqllibrary;

	static void GetTableCache(GarrysMod::Lua::ILuaBase* LUA, const char* name) {
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_REG); // +1
		LUA->GetField(-1, "gluamysql"); // +2
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(1); // +1

			LUA->CreateTable(); // +2
			LUA->Push(-1); // +3
			LUA->SetField(-3, "gluamysql"); // +2
		}

		// +2

		LUA->GetField(-1, name); // +3

		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(1); // +2

			LUA->CreateTable(); // +3
			LUA->CreateTable(); // +4 
			LUA->PushString("v"); // +5
			LUA->SetField(-2, "__mode"); // +4
			LUA->SetMetaTable(-2); // +3

			LUA->Push(-1); // +4
			LUA->SetField(-3, name); // +3
		}

		// +3

		LUA->Remove(LUA->Top() - 1); // +2
		LUA->Remove(LUA->Top() - 1); // +1
	}

	static void GetUserDataTable(GarrysMod::Lua::ILuaBase* LUA, int place) {
		if (place < 0) {
			place = LUA->Top() + place + 1;
		}

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_REG); // +1
		LUA->GetField(-1, "gluamysql_data"); // +2
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(1); // +1

			LUA->CreateTable(); // +2
			LUA->CreateTable(); // +3
			LUA->PushString("k"); // +4
			LUA->SetField(-2, "__mode"); // +3
			LUA->SetMetaTable(-2); // +2
			LUA->Push(-1); // +3
			LUA->SetField(-3, "gluamysql_data"); // +2
		}

		// +1

		LUA->Push(place); // +2
		LUA->GetTable(-2); // +2

		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(1); // +1

			LUA->CreateTable(); // +2

			LUA->Push(place); // +3
			LUA->Push(-2); // +4
			LUA->SetTable(-4); // +2
		}

		// +2

		LUA->Remove(LUA->Top() - 1); // +1
	}

	static void DeleteUserDataTable(GarrysMod::Lua::ILuaBase* LUA, int place) {
		if (place < 0) {
			place = LUA->Top() + place;
		}
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_REG); // +1
		LUA->GetField(-1, "gluamysql_data"); // +2
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(2); // 0
			return;
		}

		// +1
		LUA->Push(place); // +2
		LUA->PushNil(); // +3
		LUA->SetTable(-3); // +1

		LUA->Pop(1); // 0
	}

	static void PushHookRun(GarrysMod::Lua::ILuaBase* LUA) {
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Run");
		LUA->Remove(LUA->Top() - 1);
		LUA->Remove(LUA->Top() - 1);
	}
}
