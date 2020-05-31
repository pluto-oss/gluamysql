#include "lua.hpp"
#include "mysql.h"
#include "luaactionasync.h"
#include <string>

namespace gluamysql {
	class ConnectAction : public LuaActionAsync {
	public:
		ConnectAction(lua_State* L, const char* _host, const char* _user, const char* _passwd, const char* _db, unsigned int _port = 3306) : LuaActionAsync{ L }, host(_host), user(_user), passwd(_passwd), dbname(_db), port(_port) {
			out = nullptr;
		}

		int Start(lua_State* L, LuaDatabase* db) override {
			return mysql_real_connect_start(&out, db->instance, host.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, NULL, 0);
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return mysql_real_connect_cont(&out, db->instance, db->socket_state);
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
			if (!out) {
				PushReject(L);
				lua_pushstring(L, mysql_error(db->instance));
				lua_call(L, 1, 0);
			}
			else {
				PushResolve(L);
				db->Push(L);
				lua_call(L, 1, 0);
			}
		}

		MYSQL* out;
		std::string host, user, passwd, dbname;
		unsigned int port;
	};
}