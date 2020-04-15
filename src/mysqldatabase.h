#pragma once

#include "GarrysMod/Lua/LuaBase.h"
#include "gmodmysql.h"
#include <memory>
#include <deque>
#include <map>

namespace gmodmysql {
	namespace UserDatas {
		extern int Database;
	}

	namespace MySQLDatabase {

		class Instance : public std::shared_ptr<MYSQL> {
		public:
			using std::shared_ptr<MYSQL>::shared_ptr;

			void InsertAction(IAction* action);
		};

		extern std::map<Instance, std::deque<IAction*>> action_map;

		using UserData = Instance *;
			
		extern void PushUserData(GarrysMod::Lua::ILuaBase* LUA, Instance db);
		extern int CheckStatus(IAction *item, Instance db, int waiting_state);
		extern int GetSocketStatus();
	}


	extern library mysqldatabase;
}
