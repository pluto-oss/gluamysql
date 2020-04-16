#pragma once

#include "action.h"
#include "mysqldatabase.h"

namespace gluamysql {
	class MySQLClose : public IAction {
		bool Start() override {
			last_status = mysql_close_start(db.get());
		}

		bool Poll() override {
			if (!MySQLDatabase::CheckStatus(this, db, last_status))
				return false;

			last_status = mysql_close_cont(db.get(), last_status);

			return last_status == 0;
		}

		bool Finish(GarrysMod::Lua::ILuaBase* LUA) override {
			gluamysql::MySQLDatabase::PushUserData(LUA, db); // +1
			gluamysql::GetUserDataTable(LUA, -1); // +2

			LUA->GetField(-1, "OnClose"); // +3

			if (LUA->IsType(-1, GarrysMod::Lua::Type::Function)) {
				LUA->Push(-3); // +4
				LUA->Call(1, 0); // +2

				LUA->Pop(2); // 0
			}
			else
				LUA->Pop(3); // 0

			return true;
		}

	protected:
		MySQLDatabase::Instance db;
		int last_status;
	};

}