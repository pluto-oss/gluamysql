#pragma once
#include "lua.hpp"
#include "GarrysMod/Lua/Interface.h"
#include "mysql.h"
#include <string>

namespace gluamysql {
	struct _library {
		const char *name;
		lua_CFunction func;
	};

	extern const _library library[];

	static void PushLibrary(lua_State* L, const _library* lib) {
		lua_newtable(L);
		while (lib->name) {
			lua_pushcfunction(L, lib->func);
			lua_setfield(L, -2, lib->name);

			lib++;
		}
	}

	static int CreateMetaTable(lua_State* L, const char* name, const _library* lib) {
		auto LUA = L->luabase;
		LUA->SetState(L);

		int ret = LUA->CreateMetaTable(name);

		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		while (lib->name) {
			lua_pushcfunction(L, lib->func);
			lua_setfield(L, -2, lib->name);

			lib++;
		}

		lua_pop(L, 1);

		return ret;
	}

	static void PushField(lua_State* L, std::string field, int field_type) {
		switch (field_type) {
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
			lua_pushnumber(L, std::atof(field.c_str()));
			break;
		case MYSQL_TYPE_BIT:
			lua_pushnumber(L, static_cast<int>(field[0]));
			break;
		case MYSQL_TYPE_NULL:
			lua_pushnil(L);
			break;
		default:
			lua_pushlstring(L, field.c_str(), field.length());
			break;
		}
	}

	static void PushRow(lua_State *L, MYSQL_RES* results, MYSQL_ROW row, unsigned long *lengths, unsigned int fieldcount) {
		lua_newtable(L);

		for (unsigned int i = 0; i < fieldcount; i++) {
			auto field = mysql_fetch_field_direct(results, i);

			lua_pushlstring(L, field->name, field->name_length);

			// push data from field result set
			if (lengths[i] == 0) {
				lua_pushnil(L);
			}
			else {
				PushField(L, std::string(row[i], lengths[i]), field->type);
			}

			lua_rawset(L, -3);
		}
	}
}
