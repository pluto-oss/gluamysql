#include "mysqldatabase.h"
#include <cstdint>
#include "GarrysMod/Lua/Interface.h"
#include "mysqlquery.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <poll.h>
#endif

#include "luauserdata.h"

MYSQL_GENERIC_META_FUNCTIONS(MySQLDatabase);

LUA_FUNCTION(mysqldatabase__query) {
	auto db = gluamysql::MySQLDatabase::FromUserData(LUA, 1);

	LUA->CheckString(2);
	unsigned int outlen;
	const char *str = LUA->GetString(2, &outlen);
	auto query = new gluamysql::MySQLQuery::Instance(LUA, *db, str, outlen);
	db->InsertAction(LUA, query);

	gluamysql::MySQLQuery::PushUserData(LUA, query);
	return 1;
}

gluamysql::library gluamysql::MySQLDatabase::Library = {
	{ "__gc", MySQLDatabase__gc },
	{ "__index", MySQLDatabase__index },
	{ "__newindex", MySQLDatabase__newindex },
	{ "query", mysqldatabase__query },
	{ nullptr, nullptr }
}; 

MYSQL_LUA_ACCESSORS(MySQLDatabase, true);

int GetStatus(gluamysql::IAction* item, gluamysql::MySQLDatabase::Instance db, int waiting_state) {

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

bool gluamysql::MySQLDatabase::CheckStatus(gluamysql::IAction* item, gluamysql::MySQLDatabase::Instance db, int waiting_state) {
	return waiting_state == 0 || (waiting_state & GetStatus(item, db, waiting_state)) != 0;
}


int gluamysql::MySQLDatabase::GetSocketStatus() {
#ifndef _WIN32
	return 0;
#else
	return WSAGetLastError();
#endif
}
std::map<gluamysql::MySQLDatabase::Instance, gluamysql::MySQLDatabase::action_pair> gluamysql::MySQLDatabase::action_map;

void gluamysql::MySQLDatabase::Instance::InsertAction(GarrysMod::Lua::ILuaBase *LUA, IAction* action) {
	int& reference = action_map[*this].reference;
	std::deque<gluamysql::IAction*>& list = action_map[*this].actions;
	
	if (reference == 0) {
		gluamysql::MySQLDatabase::PushUserData(LUA, this);
		reference = LUA->ReferenceCreate();
	}

	list.push_back(action);
}