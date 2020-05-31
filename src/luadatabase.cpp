#include "luadatabase.h"
#include "luaaction.h"

#include "mysql.h"
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <poll.h>
#endif

using namespace gluamysql;


int gluamysql::LuaDatabase::GetSocketStatus() {
	int waiting_state = this->socket_state;
#ifdef _WIN32
	fd_set rs, ws, es;
	int res;
	my_socket s = mysql_get_socket(this->instance);
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	FD_ZERO(&es);
	if (waiting_state & MYSQL_WAIT_READ)
		FD_SET(s, &rs);
	if (waiting_state & MYSQL_WAIT_WRITE)
		FD_SET(s, &ws);
	if (waiting_state & MYSQL_WAIT_EXCEPT)
		FD_SET(s, &es);

	res = select(1, &rs, &ws, &es, 0);
	if (res == 0)
		return MYSQL_WAIT_TIMEOUT;
	else if (res == SOCKET_ERROR)
	{
		/*
		  In a real event framework, we should handle errors and re-try the select.
		*/
		return MYSQL_WAIT_TIMEOUT;
	}
	else
	{
		int status = 0;
		if (FD_ISSET(s, &rs))
			status |= MYSQL_WAIT_READ;
		if (FD_ISSET(s, &ws))
			status |= MYSQL_WAIT_WRITE;
		if (FD_ISSET(s, &es))
			status |= MYSQL_WAIT_EXCEPT;
		return status;
	}
#else
	struct pollfd pfd;
	int timeout;
	int res;

	pfd.fd = mysql_get_socket(db.get());
	pfd.events =
		(waiting_state & MYSQL_WAIT_READ ? POLLIN : 0) |
		(waiting_state & MYSQL_WAIT_WRITE ? POLLOUT : 0) |
		(waiting_state & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);

	res = poll(&pfd, 1, 0);
	if (res == 0)
		return MYSQL_WAIT_TIMEOUT;
	else if (res < 0)
	{
		/*
		  In a real event framework, we should handle EINTR and re-try the poll.
		*/
		return MYSQL_WAIT_TIMEOUT;
	}
	else
	{
		int status = 0;
		if (pfd.revents & POLLIN)
			status |= MYSQL_WAIT_READ;
		if (pfd.revents & POLLOUT)
			status |= MYSQL_WAIT_WRITE;
		if (pfd.revents & POLLPRI)
			status |= MYSQL_WAIT_EXCEPT;
		return status;
	}
#endif
}

int LuaDatabase::Tick(lua_State* L) {
	auto db = Get(L, 1);

	if (db->current_action == nullptr && db->queue.size() > 0) {
		auto it = db->queue.begin();
		db->current_action = it[0];
		db->queue.erase(it);
	}

	while (db->current_action != nullptr) {
		auto& action = db->current_action;
		if (action->Query(L, db)) {
			action->Finish(L, db);
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

	return LUA->GetUserType<LuaDatabase>(index, LuaDatabase::MetaType);
}

void LuaUserData<LuaDatabase>::PushLuaUserData(lua_State* L, LuaDatabase* what) {
	auto LUA = L->luabase;
	LUA->SetState(L);

	return LUA->PushUserType(what, LuaDatabase::MetaType);
}