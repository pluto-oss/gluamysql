#include "luaactionasync.h"
#include "luadatabase.h"
#include "lua.hpp"
#include "mysql.h"

namespace gluamysql {
	class AutoCommitAction : public LuaActionAsync {
	public:
		AutoCommitAction(lua_State* L, bool _autocommit) : LuaActionAsync{ L }, autocommit(_autocommit) {
		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_autocommit_start(&ret, db->instance, autocommit);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_autocommit_cont(&ret, db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (ret != 0) {
				Reject(L, db);
				return;
			}
			PushResolve(L);
			lua_call(L, 0, 0);
		}

	public:
		bool autocommit;

		my_bool ret = 0;
	};

	class CommitAction : public LuaActionAsync {
	public:
		CommitAction(lua_State* L) : LuaActionAsync{ L } {
		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_commit_start(&ret, db->instance);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_commit_cont(&ret, db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (ret != 0) {
				Reject(L, db);
				return;
			}
			PushResolve(L);
			lua_call(L, 0, 0);
		}

	public:
		my_bool ret = 0;
	};

	class RollbackAction : public LuaActionAsync {
	public:
		RollbackAction(lua_State* L) : LuaActionAsync{ L } {
		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_rollback_start(&ret, db->instance);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_rollback_cont(&ret, db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (ret != 0) {
				Reject(L, db);
				return;
			}
			PushResolve(L);
			lua_call(L, 0, 0);
		}

	public:
		my_bool ret = 0;
	};
}