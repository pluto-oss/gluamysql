#pragma once

#include "lua.hpp"
#include "GarrysMod/Lua/Interface.h"
#include "mysql.h"
#include "gluamysql.h"
#include "luauserdata.h"
#include "luaactionasync.h"
#include <string>

namespace gluamysql {
	class LuaPreparedStatement;
	class PrepareStatementAction;

	template <>
	class LuaUserData<LuaPreparedStatement> {
	public:
		static LuaPreparedStatement* GetLuaUserData(lua_State* L, int index);
		static void PushLuaUserData(lua_State* L, LuaPreparedStatement* what);
	};

	class LuaPreparedStatement {
	public:
		LuaPreparedStatement(lua_State* L, LuaDatabase *_db, std::string statement) : db(_db) {
			stmt = mysql_stmt_init(_db->instance);
			
			Start(L, statement);
		}

	private:
		void Start(lua_State* L, std::string statemenht);

	public:
		void Push(lua_State* L);
		void PushReference(lua_State* L) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
			lua_getfield(L, -1, "db");
			lua_remove(L, lua_gettop(L) - 1);
		}

	public:
		std::shared_ptr<PrepareStatementAction> action;
		MYSQL_STMT* stmt;
		LuaDatabase* db;
		int reference = LUA_REFNIL;


	public:
		static const char* MetaName;
		static int MetaType;

		static const _library library[];
	};
}