#ifndef GAMED_GS_EVENTSYS_SRC_EV_LUA_ENT_H_
#define GAMED_GS_EVENTSYS_SRC_EV_LUA_ENT_H_

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/lua/lua_engine.h"


namespace gamed {

class EvLuaEntityLockGuard;

///
/// EvLuaEntity
///
class EvLuaEntity : shared::noncopyable
{
	friend class EvLuaEntityLockGuard;
public:
	EvLuaEntity();
	~EvLuaEntity();

	bool Init(const luabind::CFunction* func_table, int32_t table_size);
	bool LoadFile(const char* file);

protected:
	luabind::LuaEngine* Attach();
	void Detach();

private:
	bool is_attached_;
	shared::MutexLock  mutex_lua_state_;
	luabind::LuaEngine lua_engine_;
};

///
/// EvLuaEntityLockGuard
///
class EvLuaEntityLockGuard
{
public:
	explicit EvLuaEntityLockGuard(EvLuaEntity& ent)
		: entity_(ent),
		  lua_engine_(NULL)
	{
		lua_engine_ = entity_.Attach();
	}

	~EvLuaEntityLockGuard()
	{
		entity_.Detach();
	}

	inline luabind::LuaEngine* lua_engine();

private:
	EvLuaEntity& entity_;
	luabind::LuaEngine* lua_engine_;
};

///
/// inline func
///
inline luabind::LuaEngine* EvLuaEntityLockGuard::lua_engine()
{
	return lua_engine_;
}

#define EvLuaEntityLockGuard(x) SHARED_STATIC_ASSERT(false); //missing guard var name

} // namespace gamed

#endif // GAMED_GS_EVENTSYS_SRC_EV_LUA_ENT_H_
