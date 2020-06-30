#include "luadatabase.h"
#include "actions/close.h"
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

std::deque<LuaDatabase*>LuaDatabase::open_databases = std::deque<LuaDatabase *>();

int gluamysql::LuaDatabase::GetSocketStatus() {
	return GetCurrentDatabaseStatus(instance, socket_state);
}

int gluamysql::GetCurrentDatabaseStatus(MYSQL* instance, int status) {
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

void LuaDatabase::RunTick(lua_State *L) {
	do {
		if (current_action == nullptr) {
			if (queue.size() == 0) {
				break;
			}
			auto it = queue.begin();
			current_action = it[0];
			queue.erase(it);
		}

		if (current_action->has_finished) {
			// errored in DoFinish
			current_action->Free(L);
		}
		else if (current_action->Query(L, this)) {
			current_action->DoFinish(L, this);
			current_action->Free(L);
		}
		else {
			break;
		}

		// set up next action
		current_action = nullptr;
	} while (current_action != nullptr);

	if (queue.size() == 0 && current_action == nullptr && queue_reference != LUA_REFNIL && queue_reference != LUA_NOREF) {
		luaL_unref(L, LUA_REGISTRYINDEX, queue_reference);
		queue_reference = LUA_REFNIL;
	}

}

LuaDatabase* LuaUserData<LuaDatabase>::GetLuaUserData(lua_State* L, int index, bool ignore_null) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	auto ret = LUA->GetUserType<LuaDatabase>(index, LuaDatabase::MetaType);

	if (!ignore_null && !ret) {
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