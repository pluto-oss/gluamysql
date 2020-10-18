#pragma once

#include "lua.hpp"
#include "mysql.h"
#include "luapreparedstatement.h"
#include "luaactionasync.h"
#include <string>
#include <vector>
#include <memory>
#include <tuple>

namespace gluamysql {
	class ExecuteStatementAction : public LuaAction {
	private:
		class ResultData {
			struct ROW_CONTAINER {
				MYSQL_ROW row;
				unsigned int columns;
			};
		public:
			ResultData(MYSQL_STMT *stmt) {
				results = mysql_stmt_result_metadata(stmt);
				auto columns = mysql_stmt_field_count(stmt);
				row_container = std::shared_ptr<ROW_CONTAINER>(new ROW_CONTAINER, [](ROW_CONTAINER *row) {
					if (!row) {
						return;
					}
						
					for (unsigned int i = 0; i < row->columns; i++) {
						auto& data = row->row[i];
						if (data) {
							delete[] data;
						}
					}

					delete[] row->row;
				});
				row_container->row = new char* [columns];
				row_container->columns = columns;

				binds.resize(columns);
				is_nulls.resize(columns);
				lengths.resize(columns);

				MYSQL_FIELD* fields = mysql_fetch_fields(results);
				for (unsigned int i = 0; i < columns; i++) {
					auto length = fields[i].length + 1;
					row_container->row[i] = new char[length];
					auto& bind = binds[i];
					bind.buffer = row_container->row[i];

					bind.buffer_type = MYSQL_TYPE_STRING;
					bind.buffer_length = length;
					bind.length = &lengths[i];
					lengths[i] = 0;
					bind.is_null = &is_nulls[i];
					is_nulls[i] = 1;
					bind.is_unsigned = 0;
				}

				if (mysql_stmt_bind_result(stmt, binds.data()) != 0) {
					throw "Failed to Bind";
				}
			}

		public:
			std::shared_ptr<ROW_CONTAINER> row_container;
			std::vector<MYSQL_BIND> binds;
			std::vector<my_bool> is_nulls;
			std::vector<unsigned long> lengths;
			
			MYSQL_RES* results;
		};

	public:
		ExecuteStatementAction(lua_State* L, int start, LuaPreparedStatement* _stmt) : LuaAction{ L }, stmt(_stmt) {
			arguments.resize(mysql_stmt_param_count(stmt->stmt));
			for (auto& argument : arguments) {
				argument.is_null = CreateBuffer<my_bool>(1);
			}

			for (int i = start; i <= lua_gettop(L) && i <= start + arguments.size() - 1; i++) {
				PopulateBind(L, &arguments[i - start], i);
			}

			current_action = std::make_tuple(ExecuteStart, ExecuteContinue, ExecuteFinish);

			lua_newtable(L);
			reference = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		template <typename T>
		T *CreateBuffer(T data) {
			auto buf = std::string((const char *)&data, sizeof(data));
			buffers.push_back(buf);
			return (T*)buffers.back().c_str();
		}

		void PopulateBind(lua_State* L, MYSQL_BIND* what, int where) {
			auto type = lua_type(L, where);
			switch (type) {
			case LUA_TBOOLEAN:
				*what->is_null = 0;
				what->buffer_type = MYSQL_TYPE_TINY;
				what->buffer = CreateBuffer<signed char>(static_cast<signed char>(lua_toboolean(L, where)));
				break;

			case LUA_TNUMBER:
				*what->is_null = 0;
				what->buffer_type = MYSQL_TYPE_DOUBLE;
				what->buffer = CreateBuffer<double>(lua_tonumber(L, where));
				break;

			case LUA_TSTRING:
				*what->is_null = 0;
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
				*what->is_null = 1;
				break;
			default:
				luaL_typerror(L, where, "string/number/boolean");
				break;
			}
		}

		using StartAction = int (*)(ExecuteStatementAction*, lua_State*, LuaDatabase*);
		using ContinueAction = int (*)(ExecuteStatementAction*, lua_State*, LuaDatabase*);
		using FinishAction = bool (*)(ExecuteStatementAction*, lua_State*, LuaDatabase*);

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

			bool has_finished = false;

			if (db->socket_state == 0) {
				has_finished = std::get<2>(current_action)(this, L, db);
				started = false;
			}

			return has_finished;
		}

		static int ExecuteStart(ExecuteStatementAction *action, lua_State* L, LuaDatabase* db) {
			if ((action->out = mysql_stmt_bind_param(action->stmt->stmt, action->arguments.data())) != 0) {
				return 0;
			}
			return mysql_stmt_execute_start(&action->out, action->stmt->stmt);
		}
		static int ExecuteContinue(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_execute_cont(&action->out, action->stmt->stmt, db->socket_state);
		}
		static bool ExecuteFinish(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			if (action->out != 0) {
				action->Reject(L, db, mysql_stmt_error(action->stmt->stmt));
				return true;
			}

			action->current_action = std::make_tuple(StoreStart, StoreContinue, StoreFinish);

			return false;
		}


		static int StoreStart(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_store_result_start(&action->out, action->stmt->stmt);
		}
		static int StoreContinue(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_store_result_cont(&action->out, action->stmt->stmt, db->socket_state);
		}
		static bool StoreFinish(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			if (action->out != 0) {
				action->Reject(L, db, mysql_stmt_error(action->stmt->stmt));
				return true;
			}

			// binds results
			if (mysql_stmt_result_metadata(action->stmt->stmt) != nullptr) {
				try {
					action->resultdata = std::make_shared<ResultData>(action->stmt->stmt);
				}
				catch (const char*) {
					action->Reject(L, db);
					action->current_action = std::make_tuple(FreeResultsStart, FreeResultsContinue, FreeResultsFinish);
					return true;
				}
			}
			action->current_action = std::make_tuple(FetchStart, FetchContinue, FetchFinish);

			return false;
		}


		static int FetchStart(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_fetch_start(&action->out, action->stmt->stmt);
		}
		static int FetchContinue(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_fetch_cont(&action->out, action->stmt->stmt, db->socket_state);
		}
		static bool FetchFinish(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			if (action->out == MYSQL_DATA_TRUNCATED) {
				action->Reject(L, db, "MySQL data truncated");
				action->current_action = std::make_tuple(FreeResultsStart, FreeResultsContinue, FreeResultsFinish);
				return false;
			}
			else if (action->out == MYSQL_NO_DATA) {
				action->current_action = std::make_tuple(FreeResultsStart, FreeResultsContinue, FreeResultsFinish);
				return false;
			}
			else if (action->out != 0) {
				action->Reject(L, db, mysql_stmt_error(action->stmt->stmt));
				action->current_action = std::make_tuple(FreeResultsStart, FreeResultsContinue, FreeResultsFinish);
				return false;
			}

			// more data
			lua_rawgeti(L, LUA_REGISTRYINDEX, action->reference);

			if (action->resultdata) {
				gluamysql::PushRow(L, action->resultdata->results, action->resultdata->row_container->row, action->resultdata->lengths.data(), action->resultdata->row_container->columns);
			}
			auto id = mysql_insert_id(db->instance);
			if (id != 0) {
				// was inserted
				lua_pushinteger(L, (lua_Integer)id);
				lua_setfield(L, -2, "LAST_INSERT_ID");
			}

			lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
			lua_pop(L, 1);

			return false;
		}

		
		static int FreeResultsStart(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			action->fetch_out = 0;
			if (action->resultdata == nullptr) {
				return 0;
			}
			return mysql_free_result_start(action->resultdata->results);
		}
		static int FreeResultsContinue(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_free_result_cont(action->resultdata->results, db->socket_state);
		}
		static bool FreeResultsFinish(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			action->current_action = std::make_tuple(FreeStatementStart, FreeStatementContinue, FreeStatementFinish);
			return false;
		}


		static int FreeStatementStart(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_free_result_start(&action->fetch_out, action->stmt->stmt);
		}
		static int FreeStatementContinue(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return mysql_stmt_free_result_cont(&action->fetch_out, action->stmt->stmt, db->socket_state);
		}
		static bool FreeStatementFinish(ExecuteStatementAction* action, lua_State* L, LuaDatabase* db) {
			return true;
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			PushResolve(L);
			lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
			lua_call(L, 1, 0);
		}

		void Free(lua_State* L) override {
			LuaAction::Free(L);

			luaL_unref(L, LUA_REGISTRYINDEX, reference);
		}

	public:
		bool started = false;
		Action current_action;

	public:
		int out = 0;
		std::string statement;
		LuaPreparedStatement* stmt;

		std::vector<MYSQL_BIND> arguments;
		std::deque<std::string> buffers;

		int reference = LUA_REFNIL;

		my_bool fetch_out = 0;

		std::shared_ptr<ResultData> resultdata = nullptr;
	};
}