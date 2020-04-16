#pragma once

#include "GarrysMod/Lua/LuaBase.h"
#include "gluamysql.h"
#include <memory>
#include <deque>
#include <map>

namespace gluamysql {
	namespace UserDatas {
		extern int MySQLDatabase;
	}

	namespace MySQLDatabase {

		class Instance : public std::shared_ptr<MYSQL> {
		public:
			using std::shared_ptr<MYSQL>::shared_ptr;

			void InsertAction(GarrysMod::Lua::ILuaBase* LUA, IAction* action);
		};

		struct action_pair {
			std::deque<IAction*> actions;
			int reference;
		};
		extern std::map<Instance, action_pair> action_map;

		extern void PushUserData(GarrysMod::Lua::ILuaBase* LUA, Instance *db);
		extern Instance *FromUserData(GarrysMod::Lua::ILuaBase* LUA, int place);
		extern bool CheckStatus(IAction *item, Instance db, int waiting_state);
		extern int GetSocketStatus();

		extern library Library;
	}
}
