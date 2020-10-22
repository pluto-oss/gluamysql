#include "luaactionasync.h"
#include "lua.hpp"
#include "mysql.h"
#include "luapreparedstatement.h"
#include <algorithm>

namespace gluamysql {

	class PrepareStatementCloseAction : public LuaActionAsync {
	public:
		PrepareStatementCloseAction(lua_State* L, LuaPreparedStatement* _stmt) : LuaActionAsync{ L }, stmt(_stmt->stmt) {
		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_close_start(&ret, stmt);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_stmt_close_cont(&ret, stmt, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			db->stmts.erase(std::remove(db->stmts.begin(), db->stmts.end(), stmt));
		}

	public:
		my_bool ret;

		MYSQL_STMT* stmt;
	};

}