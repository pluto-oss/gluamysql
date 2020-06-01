#pragma once

#include "lua.hpp"
#include "mysql.h"
#include "luaactionasync.h"
#include <string>
#include <cstdlib>
#include <tuple>

namespace gluamysql {
	static void PushFieldToLua(lua_State* L, std::string field, int field_type) {
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

	class QueryAction : public LuaAction {
	public:
		QueryAction(lua_State* L, std::string _query) : LuaAction{ L }, query(_query) {
			lua_newtable(L);
			data_reference = luaL_ref(L, LUA_REGISTRYINDEX);

			current_action = std::make_tuple(StartQuery, ContinueQuery, FinishQuery);
		}

		using StartAction = int (*)(QueryAction*, lua_State*, LuaDatabase*);
		using ContinueAction = int (*)(QueryAction*, lua_State*, LuaDatabase*);
		using FinishAction = bool (*)(QueryAction*, lua_State *, LuaDatabase *);

		using Action = std::tuple<StartAction, ContinueAction, FinishAction>;

		bool Query(lua_State* L, LuaDatabase* db) override {
			int state_needed = db->socket_state;
			db->socket_state = db->GetSocketStatus();
			if (!started) {
				started = true;
				db->socket_state = std::get<0>(current_action)(this, L, db);
			}
			
			if ((state_needed & db->socket_state) != 0) {
				db->socket_state = std::get<1>(current_action)(this, L, db);
			}

			if (db->socket_state == 0) {
				has_finished = std::get<2>(current_action)(this, L, db);
				started = false;
			}

			return has_finished;
		}

		static int StartQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_query_start(&action->out, db->instance, action->query.c_str());
		}
		static int ContinueQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_query_cont(&action->out, db->instance, db->socket_state);
		}
		static bool FinishQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			if (action->out == 0) {
				action->current_action = std::make_tuple(StartStore, ContinueStore, FinishStore);
				return false;
			}
			action->Reject(L, db);
			return true;
		}

		static int StartStore(QueryAction *action, lua_State* L, LuaDatabase* db) {
			return mysql_store_result_start(&action->results, db->instance);
		}
		static int ContinueStore(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_store_result_cont(&action->results, db->instance, db->socket_state);
		}
		static bool FinishStore(QueryAction* action, lua_State *L, LuaDatabase *db) {
			if (mysql_errno(db->instance) != 0) {
				action->Reject(L, db);
				return true;
			}

			// no results from query
			if (!action->results)
				return true;

			action->current_action = std::make_tuple(StartFetch, ContinueFetch, FinishFetch);
			return false;
		}

		static int StartFetch(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_fetch_row_start(&action->row, action->results);
		}
		static int ContinueFetch(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_fetch_row_cont(&action->row, action->results, db->socket_state);
		}
		static bool FinishFetch(QueryAction* action, lua_State *L, LuaDatabase *db) {
			if (action->row == nullptr) {
				// no more rows? check for error!!!

				action->current_action = std::make_tuple(StartFreeResult, ContinueFreeResult, FinishFreeResult);
				return false;
			}

			lua_rawgeti(L, LUA_REGISTRYINDEX, action->data_reference);
			lua_newtable(L);

			auto lengths = mysql_fetch_lengths(action->results);

			for (unsigned int i = 0; i < mysql_num_fields(action->results); i++) {
				auto field = mysql_fetch_field_direct(action->results, i);

				lua_pushlstring(L, field->name, field->name_length);

				// push data from field result set
				if (lengths[i] == 0) {
					lua_pushnil(L);
				}
				else {
					gluamysql::PushFieldToLua(L, std::string(action->row[i], lengths[i]), field->type);
				}

				lua_rawset(L, -3);
			}

			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);


			// keep same action looping until done
			return false;
		}

		static int StartFreeResult(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_free_result_start(action->results);
		}
		static int ContinueFreeResult(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_free_result_cont(action->results, db->socket_state);
		}
		static bool FinishFreeResult(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return true;
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			PushResolve(L);
			lua_rawgeti(L, LUA_REGISTRYINDEX, data_reference);
			lua_call(L, 1, 0);
		}

		void Free(lua_State* L) override {
			LuaAction::Free(L);

			luaL_unref(L, LUA_REGISTRYINDEX, data_reference);
		}

	public:
		bool started;
		Action current_action;
		bool has_finished = false;

	public:
		int out = 0;
		std::string query;

		MYSQL_RES* results = nullptr;
		MYSQL_ROW row = nullptr;

		int data_reference = LUA_REFNIL;
	};
}