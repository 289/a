#include "lua_engine.h"
#include <stdio.h>

namespace luabind
{

void LuaEngine::Init(lua_State* state)
{
	state_ = (state == NULL ? luaL_newstate() : state);
	luaL_openlibs(state_);
}

void LuaEngine::Destroy()
{
	if(state_ != NULL)
	{
		lua_close(state_);
		state_ = NULL;
	}
}

void LuaEngine::RegisterCFunc(const CFunction* func_table, int32_t table_size)
{
	for(int32_t i = 0; i < table_size; ++i)
	{
		lua_register(state_, func_table[i].funcname, func_table[i].func);
	}
}

bool LuaEngine::LoadFile(const char* file)
{
	return luaL_dofile(state_, file) ? Error() : true;
}

bool LuaEngine::LoadString(const char* str)
{
	return luaL_dostring(state_, str) ? Error() : true;
}

bool LuaEngine::Call(const char* method, const LuaValueArray* args)
{
	lua_settop(state_, 0);
	lua_getglobal(state_, method);
	int nargs = (args == NULL ? 0 : args->size());
	for(int i = 0; i < nargs; ++i)
	{
		PushValue(args->at(i));
	}
	return lua_pcall(state_, nargs, LUA_MULTRET, 0) ? Error() : true;
}

bool LuaEngine::Call(const char* file, const char* method, const LuaValueArray* args)
{
	lua_settop(state_, 0);
	if(!LoadFile(file))
	{
		return false;
	}

	lua_getglobal(state_, method);
	int nargs = (args == NULL ? 0 : args->size());
	for(int i = 0; i < nargs; ++i)
	{
		PushValue(args->at(i));
	}
	return lua_pcall(state_, nargs, LUA_MULTRET, 0) ? Error() : true;
}

void LuaEngine::ClearStack()
{
	lua_settop(state_, 0);
}

void LuaEngine::PushValue(const LuaValue& value)
{
	switch(value.GetType())
	{
	case LuaValue::SVT_BOOL:
		lua_pushnumber(state_, value.GetBool());
		break;
	case LuaValue::SVT_NUMBER:
		lua_pushnumber(state_, value.GetDouble());
		break;
	case LuaValue::SVT_STRING:
		lua_pushstring(state_, value.GetString().c_str());
		break;
	case LuaValue::SVT_INT64:
		{
			int64_t id = value.GetInt64();
			std::string str64((char*)&id, sizeof(id));
			lua_pushlstring(state_, str64.c_str(), str64.length());
		}
		break;
	default:
		break;
	}
}

bool LuaEngine::Error()
{
	if(lua_isstring(state_, -1))
	{
		printf("%s\n", lua_tostring(state_, -1));
	}
	lua_pop(state_, 1);
	return false;
}

void LuaEngine::StackDump()
{
	int top = lua_gettop(state_);
	for(int i = 1; i <= top; i++)
	{
		int t = lua_type(state_, i);
		switch(t)
		{
		case LUA_TSTRING:
			printf("'%s'", lua_tostring(state_, i));
			break;
		case LUA_TBOOLEAN:
			printf(lua_toboolean(state_, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			printf("%g", lua_tonumber(state_, i));
			break;
		default:
			printf("%s", lua_typename(state_, t));
			break;
		}
		printf(" ");
	}
	printf("\n");
}

} // namespace luabind
