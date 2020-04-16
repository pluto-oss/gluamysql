#pragma once

#define MYSQL_GENERIC_META_FUNCTIONS(type)\
	LUA_FUNCTION(type ## __gc) {\
		auto item = LUA->GetUserType<gluamysql:: type ::Instance *>(1, gluamysql::UserDatas:: type );\
		delete item;\
\
		return 0;\
	}\
	LUA_FUNCTION(type ## __index) {\
		gluamysql::GetUserDataTable(LUA, 1);\
\
		LUA->Push(2);\
		LUA->GetTable(-2);\
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {\
			LUA->GetMetaTable(1);\
			LUA->Push(2);\
			LUA->GetTable(-2);\
		}\
		return 1;\
	} \
	LUA_FUNCTION(type ## __newindex) {\
		gluamysql::GetUserDataTable(LUA, 1);\
\
		LUA->Push(2);\
		LUA->Push(3);\
		LUA->SetTable(-3);\
		return 0;\
	}

#define MYSQL_LUA_ACCESSORS(type, needs_new)\
	void gluamysql:: type ::PushUserData(GarrysMod::Lua::ILuaBase *LUA, gluamysql:: type ::Instance *db) {\
		gluamysql::GetTableCache(LUA, #type);\
	\
		auto ptr = reinterpret_cast<std::uintptr_t>(db->get());\
	\
		LUA->PushNumber(ptr);\
		LUA->GetTable(-2);\
	\
		if (LUA->IsType(-1, GarrysMod::Lua::Type::Nil)) {\
			LUA->Pop(1);\
			LUA->PushUserType_Value(needs_new ? new gluamysql:: type ::Instance(*db) : db, gluamysql::UserDatas:: type, true);\
			LUA->PushNumber(ptr);\
			LUA->Push(-2);\
	\
			LUA->SetTable(-4);\
		}\
	\
		LUA->Remove(LUA->Top() - 1);\
	}\
	\
	gluamysql:: type ::Instance* gluamysql:: type ::FromUserData(GarrysMod::Lua::ILuaBase* LUA, int place) {\
		return *LUA->GetUserType<gluamysql:: type ::Instance*>(place, gluamysql::UserDatas:: type);\
	}
