#include "luaactionasync.h"
#include "lua.hpp"
#include "mysql.h"

namespace gluamysql {

	class TemplateActionAsync : public LuaActionAsync {
	public:
		TemplateActionAsync(lua_State* L) : LuaActionAsync{ L } {

		}

	public:
		int Start(lua_State* L, LuaDatabase* db) override {
			return 0;
		}
		int Continue(lua_State* L, LuaDatabase* db) override {
			return 0;
		}

		void Finish(lua_State* L, LuaDatabase* db) override {
		}
	};

}