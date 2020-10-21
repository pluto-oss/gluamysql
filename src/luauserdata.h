#pragma once
#include "lua.hpp"

namespace gluamysql {
	template <typename T>
	class LuaUserData {
	public:

		static T* GetLuaUserData(lua_State* L, int index, bool ignore_null = false) {
		}

		static void PushLuaUserData(lua_State* L, T* what) {
		}

	};
}