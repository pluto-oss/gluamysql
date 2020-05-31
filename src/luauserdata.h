#pragma once
#include "lua.hpp"

namespace gluamysql {
	template <typename T>
	class LuaUserData {
	public:

		static T* GetLuaUserData(lua_State* L, int index) {
			lua_pushstring("LuaUserData::GetLuaUserData not implemented for class ");
			lua_pushstring(typeid(T).name());
			lua_concat(L, 2);
			lua_error(L);
		}

		static void PushLuaUserData(lua_State* L, T* what) {
			lua_pushstring("LuaUserData::PushLuaUserData not implemented for class ");
			lua_pushstring(typeid(T).name());
			lua_concat(L, 2);
			lua_error(L);
		}

	};
}