#pragma once

#include "nonblocking.h"
#include "gmodmysql.h"
#include "mysqldatabase.h"
#include "mysql.h"

namespace gmodmysql {
	class MySQLConnect : public IAction {
	public:
		MySQLConnect(MySQLDatabase::Instance _db, const char* _host, const char* _user, const char* _passwd,
			const char* _dbname, unsigned int _port, const char* _unix_socket,
			unsigned long _clientflag) :
			db(_db), host(_host), user(_user), passwd(_passwd),
			dbname(_dbname), port(_port), unix_socket(_unix_socket),
			clientflag(_clientflag), last_status(0)
		{ }

		bool Start() override {
			last_status = mysql_real_connect_start(&ret, db.get(), host, user, passwd, dbname, port, unix_socket, clientflag);
			return last_status == 0;
		}

		bool Poll() override {
			int ready = MySQLDatabase::CheckStatus(this, db, last_status);
			if ((ready & last_status) == 0)
				return false;
			last_status = mysql_real_connect_cont(&ret, db.get(), last_status);

			return last_status == 0;
		}

		bool Finish(GarrysMod::Lua::ILuaBase* LUA) override {
			// check status

			gmodmysql::PushHookRun(LUA);
			LUA->PushString("MySQLConnection");
			gmodmysql::MySQLDatabase::PushUserData(LUA, db);

			int args = 2;

			if (ret != db.get()) {
				args++;
				LUA->PushString(mysql_error(db.get()));
			}

			LUA->Call(args, 0);
			return true;
		};

	protected:
		int last_status;
		MySQLDatabase::Instance db;
		const char* host;
		const char* user;
		const char* passwd;
		const char* dbname;
		unsigned int port;
		const char* unix_socket;
		unsigned long clientflag;

		MYSQL* ret = nullptr;
	};

	class MySQLConnectSync : public MySQLConnect {
	public:
		using MySQLConnect::MySQLConnect;

		bool Start() override {
			ret = mysql_real_connect(db.get(), host, user, passwd, dbname, port, unix_socket, clientflag);

			return ret == db.get();
		}

		bool Poll() override {
			return true;
		}
	};
}