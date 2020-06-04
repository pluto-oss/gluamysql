#pragma once
#include "lua.hpp"
#include "luauserdata.h"

namespace gluamysql {
	class LuaPromise {
	public:
		LuaPromise(lua_State* L) {
			lua_getglobal(L, "Promise");
			*(decltype(this) *)lua_newuserdata(L, sizeof(this)) = this;
			lua_pushcclosure(L, Create, 1);
			lua_call(L, 1, 1);
			promise = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		virtual void Free(lua_State* L) {
			luaL_unref(L, LUA_REGISTRYINDEX, promise);
			luaL_unref(L, LUA_REGISTRYINDEX, reject);
			luaL_unref(L, LUA_REGISTRYINDEX, resolve);
		}

		virtual void PushResolve(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, resolve);
		}

		virtual void PushReject(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, reject);
		}

		virtual void Push(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, promise);
		}

	public:
		int resolve = LUA_REFNIL, reject = LUA_REFNIL, promise = LUA_REFNIL;

	private:
		static int Create(lua_State* L) {
			LuaPromise* promise = *(LuaPromise * *)lua_touserdata(L, lua_upvalueindex(1));
			lua_pushvalue(L, 1);
			promise->resolve = luaL_ref(L, LUA_REGISTRYINDEX);
			lua_pushvalue(L, 2);
			promise->reject = luaL_ref(L, LUA_REGISTRYINDEX);
			return 0;
		}
	};
}
