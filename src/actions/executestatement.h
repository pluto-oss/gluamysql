#pragma once

#include "lua.hpp"
#include "mysql.h"
#include "luapreparedstatement.h"
#include "luaactionasync.h"
#include <string>
#include <vector>
#include <tuple>

namespace gluamysql {
	class ExecuteStatementAction : public LuaActionAsync {
	public:
		ExecuteStatementAction(lua_State* L, int start, LuaPreparedStatement* _stmt) : LuaActionAsync{ L }, stmt(_stmt) {
			arguments.resize(mysql_stmt_param_count(stmt->stmt));
			for (auto& argument : arguments) {
				argument.is_null = CreateBuffer<my_bool>(1);
			}

			for (int i = start; i <= lua_gettop(L) && i <= start + arguments.size(); i++) {
				PopulateBind(L, &arguments[i - start], i);
			}
		}

		template <typename T>
		T *CreateBuffer(T data) {
			auto buf = std::string((const char *)&data, sizeof(data));
			buffers.push_back(buf);
			return (T *)buf.c_str();
		}

		void PopulateBind(lua_State* L, MYSQL_BIND* what, int where) {
			auto type = lua_type(L, where);
			switch (type) {
			case LUA_TBOOLEAN:
				what->buffer_type = MYSQL_TYPE_TINY;
				what->buffer = CreateBuffer<signed char>(static_cast<signed char>(lua_toboolean(L, where)));
				break;
			case LUA_TNUMBER:
				what->buffer_type = MYSQL_TYPE_DOUBLE;
				what->buffer = CreateBuffer<double>(lua_tonumber(L, where));
				break;
			case LUA_TSTRING:
				what->buffer_type = MYSQL_TYPE_BLOB;
				{
					size_t length;
					const char *str = lua_tolstring(L, where, &length);
					std::string data(str, length);

					buffers.push_back(data);
					
					what->buffer = (void *)buffers.back().c_str();
					what->buffer_length = length;
					what->length = CreateBuffer<unsigned long>(length);
				}
				break;
			case LUA_TNONE:
			case LUA_TNIL:
				break;
			default:
				luaL_typerror(L, where, "string/number/boolean");
				break;
			}
		}

		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_execute_start(&out, stmt->stmt);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_execute_cont(&out, stmt->stmt, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			PushResolve(L);
			lua_call(L, 0, 0);
		}

	public:
		int out = 0;
		std::string statement;
		LuaPreparedStatement* stmt;

		std::vector<MYSQL_BIND> arguments;
		std::deque<std::string> buffers;
	};
}