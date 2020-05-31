#include "luadatabase.h"

using namespace gluamysql;

const char *LuaDatabase::MetaName = "gluamysql::LuaDatabase";

const _library LuaDatabase::library[] = {
	{ "__gc", LuaDatabase::__gc },
	{ "__tostring", LuaDatabase::__tostring },
	{ "IsValid", LuaDatabase::IsValid },
	{ 0, 0 }
};

int LuaDatabase::MetaType;