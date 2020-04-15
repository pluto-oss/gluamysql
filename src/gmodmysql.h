#pragma once

#include "GarrysMod/Lua/LuaBase.h"
#include <forward_list>
#include "nonblocking.h"
#include "mysql.h"


namespace gmodmysql {
	typedef struct function {
		const char *name;
		GarrysMod::Lua::CFunc fn;
	} function;
	using library = function[];

	extern library mysqllibrary;

	static void GetTableCache(GarrysMod::Lua::ILuaBase* LUA, const char* name) {
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_REG); // +1
		LUA->GetField(-1, "gmodmysql"); // +2
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
			LUA->Pop(1); // +1

			LUA->CreateTable(); // +2
			LUA->Push(-1); // +3
			LUA->SetField(-3, "gmodmysql"); // +2
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

	static void PushHookRun(GarrysMod::Lua::ILuaBase* LUA) {
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Run");
		LUA->Remove(LUA->Top() - 1);
		LUA->Remove(LUA->Top() - 1);
	}
}
