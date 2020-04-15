#include "mysqldatabase.h"
#include <cstdint>
#include "GarrysMod/Lua/Interface.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <poll.h>
#endif

LUA_FUNCTION(mysqldatabase__gc) {
	auto db = LUA->GetUserType<gmodmysql::MySQLDatabase::UserData>(1, gmodmysql::UserDatas::Database);
	delete db;

	return 0;
}

gmodmysql::library gmodmysql::mysqldatabase = {
	{ "__gc", mysqldatabase__gc },
	{ nullptr, nullptr }
}; 

void gmodmysql::MySQLDatabase::PushUserData(GarrysMod::Lua::ILuaBase *LUA, gmodmysql::MySQLDatabase::Instance db) {
	gmodmysql::GetTableCache(LUA, "MySQLDatabase");

	auto ptr = reinterpret_cast<std::uintptr_t>(db.get());

	LUA->PushNumber(ptr); // +2
	LUA->GetTable(-2); // +2

	if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {
		LUA->Pop(1); // +1
		LUA->PushUserType(new Instance(db), gmodmysql::UserDatas::Database); // +2
		LUA->PushNumber(ptr); // +3 
		LUA->Push(-2); // +4

		LUA->SetTable(-4); // +2
	}

	// +2

	LUA->Remove(LUA->Top() - 1); // +1
}

int gmodmysql::MySQLDatabase::CheckStatus(gmodmysql::IAction *item, gmodmysql::MySQLDatabase::Instance db, int waiting_state) {
#ifdef _WIN32
	fd_set rs, ws, es;
	int res;
	my_socket s = mysql_get_socket(db.get());
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

int gmodmysql::MySQLDatabase::GetSocketStatus() {
#ifndef _WIN32
	return 0;
#else
	return WSAGetLastError();
#endif
}
std::map<gmodmysql::MySQLDatabase::Instance, std::deque<gmodmysql::IAction *>> gmodmysql::MySQLDatabase::action_map;

void gmodmysql::MySQLDatabase::Instance::InsertAction(IAction* action) {
	action_map[*this].push_back(action);
}