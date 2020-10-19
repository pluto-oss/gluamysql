#pragma once

#include "lua.hpp"
#include "mysql.h"
#include "luaactionasync.h"
#include <string>
#include <cstdlib>
#include <tuple>

namespace gluamysql {
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
			if (!started) {
				started = true;
				db->socket_state = db->GetSocketStatus();
				db->socket_state = std::get<0>(current_action)(this, L, db);
			}
			else if (db->CheckStatus()) {
				db->socket_state = db->GetSocketStatus();
				db->socket_state = std::get<1>(current_action)(this, L, db);
			}

			if (db->socket_state == 0) {
				if (std::get<2>(current_action)(this, L, db)) {
					auto _errno = mysql_errno(db->instance);
					if (!has_retried && (_errno == 2006 || _errno == 2013)) {
						// server lost, try reperform
						has_retried = true;

						lua_newtable(L);
						lua_rawseti(L, LUA_REGISTRYINDEX, data_reference);
					}

					return true;
				}
				started = false;
			}

			return false;
		}

		static int StartQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_real_query_start(&action->out, db->instance, action->query.c_str(), action->query.length());
		}
		static int ContinueQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_real_query_cont(&action->out, db->instance, db->socket_state);
		}
		static bool FinishQuery(QueryAction* action, lua_State* L, LuaDatabase* db) {
			if (action->out != 0) {
				action->Reject(L, db);
				return true;
			}

			action->results = mysql_use_result(db->instance);

			if (!action->results) {
				if (mysql_field_count(db->instance) != 0) {
					action->Reject(L, db);
				}
				return true;
			}

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
				if (mysql_errno(db->instance) != 0) {
					action->Reject(L, db);
					return true;
				}

				action->current_action = std::make_tuple(StartFreeResult, ContinueFreeResult, FinishFreeResult);
				return false;
			}

			lua_rawgeti(L, LUA_REGISTRYINDEX, action->data_reference);

			gluamysql::PushRow(L, action->results, action->row, mysql_fetch_lengths(action->results), mysql_num_fields(action->results));

			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);

			lua_pop(L, 1);

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
			auto id = mysql_insert_id(db->instance);
			if (id != 0) {
				// was inserted
				lua_pushinteger(L, (lua_Integer)id);
				lua_setfield(L, -2, "LAST_INSERT_ID");
			}

			auto affected = mysql_affected_rows(db->instance);
			if (affected != -1) {
				lua_pushinteger(L, (lua_Integer)affected);
				lua_setfield(L, -2, "AFFECTED_ROWS");
			}
			lua_call(L, 1, 0);
		}

		void Free(lua_State* L) override {
			LuaAction::Free(L);

			luaL_unref(L, LUA_REGISTRYINDEX, data_reference);
		}

	public:
		bool started = false;
		Action current_action;

	public:
		int out = 0;
		std::string query;

		MYSQL_RES* results = nullptr;
		MYSQL_ROW row = nullptr;

		int data_reference = LUA_REFNIL;

		bool has_retried = false;
	};
}