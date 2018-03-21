#ifndef GAMED_GS_GLOBAL_GAME_LUA_ENT_H_
#define GAMED_GS_GLOBAL_GAME_LUA_ENT_H_

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/lua/lua_engine.h"


namespace gamed {

class GameLuaEntityLockGuard;

///
/// GameLuaEntity
///
class GameLuaEntity : shared::noncopyable
{
	friend class GameLuaEntityLockGuard;
public:
	GameLuaEntity();
	~GameLuaEntity();

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
/// GameLuaEntityLockGuard
///
class GameLuaEntityLockGuard
{
public:
	explicit GameLuaEntityLockGuard(GameLuaEntity& ent)
		: entity_(ent),
		  lua_engine_(NULL)
	{
		lua_engine_ = entity_.Attach();
	}

	~GameLuaEntityLockGuard()
	{
		entity_.Detach();
	}

	inline luabind::LuaEngine* lua_engine();

private:
	GameLuaEntity& entity_;
	luabind::LuaEngine* lua_engine_;
};

///
/// inline func
///
inline luabind::LuaEngine* GameLuaEntityLockGuard::lua_engine()
{
	return lua_engine_;
}

#define GameLuaEntityLockGuard(x) SHARED_STATIC_ASSERT(false); //missing guard var name

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GAME_LUA_ENT_H_
