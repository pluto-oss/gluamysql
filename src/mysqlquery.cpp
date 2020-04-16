
#include "luauserdata.h"
#include "mysqlquery.h"
#include "GarrysMod/Lua/Interface.h"

int gluamysql::UserDatas::MySQLQuery = 0;

MYSQL_GENERIC_META_FUNCTIONS(MySQLQuery);

gluamysql::library gluamysql::MySQLQuery::Library = {
	{ "__newindex", MySQLQuery__newindex},
	{ "__index", MySQLQuery__index},
	{nullptr, nullptr}
};

MYSQL_LUA_ACCESSORS(MySQLQuery, false);

bool gluamysql::MySQLQuery::Instance::Finish(GarrysMod::Lua::ILuaBase* LUA) {
	if (freeing) {
		if (!MySQLDatabase::CheckStatus(this, db, last_status))
			return false;

		last_status = mysql_free_result_cont(results, last_status);
		return last_status == 0;
	}

	if (err != 0) {
		// error in query

		LUA->ReferencePush(reference); // +1
		LUA->GetField(-1, "OnError"); // +2

		if (LUA->IsType(-1, GarrysMod::Lua::Type::Function)) {
			gluamysql::MySQLDatabase::PushUserData(LUA, &db); // +3
			LUA->PushString(mysql_error(db.get())); // +4
			LUA->Call(2, 0); // +1
			LUA->Pop(1); // 0
		}
		else
			LUA->Pop(2); // 0

		// 0

		return true;
	}

	if (result_table == 0) {
		LUA->CreateTable();
		result_table = LUA->ReferenceCreate();
	}

	do {
		if (!in_fetch) {
			last_status = mysql_fetch_row_start(&row, results);
			in_fetch = true;
		}
		else {
			last_status = mysql_fetch_row_cont(&row, results, last_status);
		}

		if (!MySQLDatabase::CheckStatus(this, db, last_status))
			return false;

		if (row != nullptr) {
			// got row, push
			LUA->ReferencePush(result_table); // +1
			LUA->PushNumber(static_cast<double>(LUA->ObjLen(-1)) + 1); // +2
			LUA->CreateTable(); // +3

			auto lengths = mysql_fetch_lengths(results);
			for (unsigned int i = 0; i < mysql_num_fields(results); i++) {
				auto field = mysql_fetch_field_direct(results, i);

				gluamysql::PushField(LUA, field, row[i], lengths[i]); // +4
				LUA->SetField(-2, field->name); // +3
			}
			// do new thing
			// +3

			LUA->ReferencePush(reference);
			LUA->GetField(-1, "OnData"); // +5
			if (LUA->IsType(-1, GarrysMod::Lua::Type::Function)) {
				LUA->Push(-2); // +6
				LUA->Push(-4); // +7
				LUA->Call(2, 0); // +4
				LUA->Pop(1); // +3
			}
			else
				LUA->Pop(2); // +3

			// +3

			// add to final result set
			LUA->SetTable(-3); // +1
			LUA->Pop(1); // 0
		}

		in_fetch = false;
	} while (row != nullptr);

	LUA->ReferencePush(reference); // +1
	LUA->GetField(-1, "OnFinish"); // +2
	if (LUA->IsType(-1, GarrysMod::Lua::Type::Function)) {
		gluamysql::MySQLDatabase::PushUserData(LUA, &db); // +3
		LUA->ReferencePush(result_table); // +4

		LUA->Call(2, 0); // +1

		LUA->Pop(1);
	}
	else
		LUA->Pop(2);


	last_status = mysql_free_result_start(results);
	freeing = true;

	return last_status == 0;
};