#pragma once

#include "action.h"
#include "gluamysql.h"
#include "mysqldatabase.h"
#include "mysql.h"
#include "action.h"
#include <cstdint>
#include <string>

namespace gluamysql {
	namespace UserDatas {
		extern int MySQLQuery;
	}

	static void PushField(GarrysMod::Lua::ILuaBase* LUA, MYSQL_FIELD* field, const char *data, unsigned long length) {
		switch (field->type) {
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
			LUA->PushNumber(atof(data));
			break;
		case MYSQL_TYPE_BIT:
			LUA->PushNumber(static_cast<int>(data[0]));
			break;
		case MYSQL_TYPE_NULL:
			LUA->PushNil();
			break;
		case MYSQL_TYPE_LONGLONG:
		default:
			LUA->PushString(data, length);
			break;
		}
	}

	namespace MySQLQuery {
		class Instance;
		extern void PushUserData(GarrysMod::Lua::ILuaBase* LUA, Instance *db);
		extern Instance* FromUserData(GarrysMod::Lua::ILuaBase* LUA, int place);

		class Instance : public IAction {
		public:
			Instance(GarrysMod::Lua::ILuaBase* LUA, MySQLDatabase::Instance _db, const char* _query, unsigned long _length) : db(_db), query(_query), length(_length) {
				err = 0;
				last_status = 0;

				gluamysql::MySQLQuery::PushUserData(LUA, this);
				gluamysql::GetUserDataTable(LUA, -1);
				reference = LUA->ReferenceCreate();
				LUA->Pop(1);
			}

			virtual ~Instance() {

			}

			bool Start() override {
				last_status = mysql_real_query_start(&err, db.get(), query, length);
				return last_status == 0;
			}

			bool Poll() override {
				if (!MySQLDatabase::CheckStatus(this, db, last_status))
					return false;

				last_status = mysql_real_query_cont(&err, db.get(), last_status);

				if (last_status == 0) {
					results = mysql_use_result(db.get());
					return true;
				}

				return false;
			}

			bool Finish(GarrysMod::Lua::ILuaBase* LUA) override;

			Instance* get() {
				return this;
			}

		protected:
			bool in_fetch = false;
			bool freeing = false;

			MYSQL_ROW row;
			MYSQL_RES *results;

			unsigned long length;
			const char* query;
			MySQLDatabase::Instance db;
			int err, last_status;

			int reference = 0;
			int result_table = 0;
		};

		extern library Library;
	}
}