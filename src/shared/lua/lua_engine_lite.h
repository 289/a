#ifndef LUABIND_LUA_INTERFACE_H
#define LUABIND_LUA_INTERFACE_H

#include "lua_engine.h"

namespace luabind
{

class LuaEngineLite
{
public:
	LuaEngineLite()
	{
		engine_.Init();
	}

	~LuaEngineLite()
	{
		engine_.Destroy();
	}

	void RegisterCFunc(const CFunction* func_table, int32_t table_size)
	{
		return engine_.RegisterCFunc(func_table, table_size);
	}
	bool LoadFile(const char* file)
	{
		return engine_.LoadFile(file);
	}
	bool LoadString(const char* str)
	{
		return engine_.LoadString(str);
	}

	void PushValue(const LuaValue& value)
	{
		return engine_.PushValue(value);
	}
	bool Call(const char* file, const char* method, const LuaValueArray* args = NULL)
	{
		return engine_.Call(file, method, args);
	}
	template <typename T>
	bool Call(const char* file, const char* method, const T* array, size_t size)
	{
		return engine_.Call(file, method, array, size);
	}
	template <typename T>
	inline int PopValue(T& value)
	{
		return engine_.PopValue(value);
	}
private:
	LuaEngine engine_;
};

} // namespace luabind

#endif // LUABIND_LUA_INTERFACE_H
