#include "ev_lua_ent.h"


namespace gamed {

using namespace shared;

///
/// EvLuaEntity
///
EvLuaEntity::EvLuaEntity()
	: is_attached_(false)
{
	lua_engine_.Init();
}

EvLuaEntity::~EvLuaEntity()
{
	MutexLockTimedGuard lock(mutex_lua_state_);
	lua_engine_.Destroy();
}

bool EvLuaEntity::Init(const luabind::CFunction* func_table, int32_t table_size)
{
	lua_engine_.RegisterCFunc(func_table, table_size);
	return true;
}

bool EvLuaEntity::LoadFile(const char* file)
{
	return lua_engine_.LoadFile(file);
}

luabind::LuaEngine* EvLuaEntity::Attach()
{
	ASSERT(!is_attached_);
	mutex_lua_state_.timed_lock();
	is_attached_ = true;
	return &lua_engine_;
}
	
void EvLuaEntity::Detach()
{
	ASSERT(is_attached_);
	is_attached_ = false;
	mutex_lua_state_.unlock();
}

} // namespace gamed
