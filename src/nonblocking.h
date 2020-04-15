#pragma once

#include "mysql.h"
#include "GarrysMod/Lua/LuaBase.h"

namespace gmodmysql {
	class IAction {
	public:
		bool TryFinish(GarrysMod::Lua::ILuaBase *LUA) {
			if (!hasran) {
				hasran = true;
				done = Start();
			}

			if (!done)
				done = Poll();
			
			if (done) {
				return Finish(LUA);
			}

			return false;
		}

		// currently ran in lua thread but not guaranteed
		virtual bool Start() = 0;
		// currently ran in lua thread but not guaranteed
		virtual bool Poll() = 0;
		// ran in lua thread
		virtual bool Finish(GarrysMod::Lua::ILuaBase* LUA) = 0;

	protected:
		bool done = false;
		bool hasran = false;
	};
}