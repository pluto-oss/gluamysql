#include "luadatabase.h"
#include "luaaction.h"

#include "mysql.h"
#ifdef _WIN32
#include <WinSock2.h>
#define poll WSAPoll
#undef POLLPRI
#define POLLPRI POLLRDBAND
#else
#include <poll.h>
#endif

using namespace gluamysql;


int gluamysql::LuaDatabase::GetSocketStatus() {
	int status = this->socket_state; 
	struct pollfd pfd;
	int timeout, res;

	pfd.fd = mysql_get_socket(instance);
	pfd.events =
		(status & MYSQL_WAIT_READ ? POLLIN : 0) |
		(status & MYSQL_WAIT_WRITE ? POLLOUT : 0) |
		(status & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);
	//if (status & MYSQL_WAIT_TIMEOUT)
	//	timeout = 1000 * mysql_get_timeout_value(instance);
	//else
	timeout = 0;
	res = poll(&pfd, 1, timeout);
	if (res == 0)
		return MYSQL_WAIT_TIMEOUT;
	else if (res < 0)
		return MYSQL_WAIT_TIMEOUT;
	else {
		int status = 0;
		if (pfd.revents & POLLIN) status |= MYSQL_WAIT_READ;
		if (pfd.revents & POLLOUT) status |= MYSQL_WAIT_WRITE;
		if (pfd.revents & POLLPRI) status |= MYSQL_WAIT_EXCEPT;
		return status;
	}
}

int LuaDatabase::Tick(lua_State* L) {
	lua_rawgeti(L, 1, 1);
	lua_getfield(L, -1, "db");
	auto db = Get(L, -1);

	if (db->current_action == nullptr && db->queue.size() > 0) {
		auto it = db->queue.begin();
		db->current_action = it[0];
		db->queue.erase(it);
	}

	while (db->current_action != nullptr) {
		auto& action = db->current_action;
		if (action->Query(L, db)) {
			action->DoFinish(L, db);
			action->Free(L);
			db->current_action = nullptr;
			
			if (db->queue.size() > 0) {
				auto it = db->queue.begin();
				db->current_action = it[0];
				db->queue.erase(it);
			}
		}
		else {
			break;
		}
	}

	return 0;
}


LuaDatabase* LuaUserData<LuaDatabase>::GetLuaUserData(lua_State* L, int index) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	auto ret = LUA->GetUserType<LuaDatabase>(index, LuaDatabase::MetaType);

	if (!ret) {
		lua_pushstring(L, "LuaDatabase is NULL");
		lua_error(L);
	}

	return ret;
}

void LuaUserData<LuaDatabase>::PushLuaUserData(lua_State* L, LuaDatabase* what) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	LUA->PushUserType(what, LuaDatabase::MetaType);
	LUA->CreateMetaTable(LuaDatabase::MetaName);
	lua_setmetatable(L, -2);
}