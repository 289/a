#ifndef LUABIND_LUA_ENGINE_H
#define LUABIND_LUA_ENGINE_H

#include <string.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "lua.hpp"
#include "lua_value.h"

namespace luabind
{

struct CFunction
{
	const char* funcname;
	lua_CFunction func;	
};

class LuaEngine
{
public:
	// 如果采用默认参数，则需要自己调用Destroy进行释放（推荐使用LuaEngine的包装类LuaEngineLite）
	// 否则不应调用Destroy，应由分配者进行释放
	void Init(lua_State* state = NULL);
	void Destroy();

	void RegisterCFunc(const CFunction* func_table, int32_t table_size);
	bool LoadFile(const char* file);
	bool LoadString(const char* str);

	void PushValue(const LuaValue& value);
	void ClearStack();

	// 需要先调用LoadFile或者LoadString加载lua脚本
	bool Call(const char* method, const LuaValueArray* args = NULL);
	template <typename T>
	bool Call(const char* method, const T* array, size_t size);

	bool Call(const char* file, const char* method, const LuaValueArray* args = NULL);
	template <typename T>
	bool Call(const char* file, const char* method, const T* array, size_t size);
	
	template <typename T>
	inline int PopValue(T& value);
private:
	template <typename T>
	inline int GetValue(int index, T& value);

	bool Error();
	void StackDump();
private:
	lua_State* state_;
};

template <typename T>
inline int LuaEngine::PopValue(T& value)
{
	int res_num = GetValue(-1, value);
	lua_pop(state_, res_num);
	return res_num;
}

template <typename T>
inline int LuaEngine::GetValue(int index, T& value)
{
	if(!lua_isnumber(state_, index))
	{
		return 0;
	}

	value = lua_tonumber(state_, index);
	return 1;
}

template <>
inline int LuaEngine::GetValue<bool>(int index, bool& value)
{
	if(!lua_isboolean(state_, index))
	{
		return 0;
	}

	value = lua_toboolean(state_, index);
	return 1;
}

template<>
inline int LuaEngine::GetValue<std::string>(int index, std::string& value)
{
	if(!lua_isstring(state_, index))
	{
		return 0;
	}

	value.assign(lua_tostring(state_, index));
	return 1;
}

template<>
inline int LuaEngine::GetValue<int64_t>(int index, int64_t& value)
{
	if(!lua_isstring(state_, index))
	{
		return 0;
	}

	size_t len = 0;
	const char* data = lua_tolstring(state_, index, &len);
	if(len > 8)
	{
		return 0;
	}
	memcpy(&value, data, len);
	return 1;
}

template <typename T>
inline bool LuaEngine::Call(const char* method, const T* array, size_t size)
{
	lua_settop(state_, 0);
	lua_getglobal(state_, method);
	lua_newtable(state_);
	for (size_t i = 0; i < size; ++i)
	{
		PushValue(LuaValue(array[i]));
		lua_rawseti(state_, -2, i + 1);
	}
	return lua_pcall(state_, 1, LUA_MULTRET, 0) ? Error() : true;
}

template <typename T>
inline bool LuaEngine::Call(const char* file, const char* method, const T* array, size_t size)
{
	lua_settop(state_, 0);
	if (!LoadFile(file))
	{
		return false;
	}

	lua_getglobal(state_, method);
	lua_newtable(state_);
	for (size_t i = 0; i < size; ++i)
	{
		PushValue(LuaValue(array[i]));
		lua_rawseti(state_, -2, i + 1);
	}
	return lua_pcall(state_, 1, LUA_MULTRET, 0) ? Error() : true;
}

} // namespace luabind

#endif // LUABIND_LUA_ENGINE_H
