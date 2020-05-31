#include "gluamysql.h"
#include "luadatabase.h"
#include "luaactionasync.h"
#include <string>

using namespace gluamysql;

class ConnectQueryTest : public LuaActionAsync {
public:
	ConnectQueryTest(lua_State* L, const char *_host, const char *_user, const char *_passwd, const char *_db, unsigned int _port = 3306) : LuaActionAsync{ L }, host(_host), user(_user), passwd(_passwd), dbname(_db), port(_port) {
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

int gluamysql::create(lua_State* L) {
	auto db = new LuaDatabase(L);
	db->Push(L);

	if (!lua_isstring(L, 1)) {
		luaL_typerror(L, 1, "string");
	}
	if (!lua_isstring(L, 2)) {
		luaL_typerror(L, 2, "string");
	}
	if (!lua_isstring(L, 3)) {
		luaL_typerror(L, 3, "string");
	}
	if (!lua_isstring(L, 4)) {
		luaL_typerror(L, 4, "string");
	}

	auto query = std::make_shared<ConnectQueryTest>(L, lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tostring(L, 4), lua_isnumber(L, 5) ? static_cast<unsigned int>(lua_tonumber(L, 5)) : 3306);

	db->InsertAction(query);
	query->Push(L);

	return 2;
}

const _library gluamysql::library[] = {
	{ "connect", gluamysql::create },
	{ 0, 0 }
};