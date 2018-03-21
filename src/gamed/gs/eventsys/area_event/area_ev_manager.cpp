#include "area_ev_manager.h"

#include "shared/base/base_define.h"
#include "shared/lua/lua_engine.h"

#include "gs/eventsys/src/ev_lua_ent.h"

#include "func_table.h"


namespace gamed {

using namespace shared;
using namespace pugi;

AreaEventManager::AreaEventManager()
	: lua_entity_rr_key_(0),
	  bt_loader_(AreaEvLibraryLink::GetBTLoader())
{
}

AreaEventManager::~AreaEventManager()
{
	for (size_t i = 0; i < evlua_entity_vec_.size(); ++i)
	{
		DELETE_SET_NULL(evlua_entity_vec_[i]);
	}
	evlua_entity_vec_.clear();
}

bool AreaEventManager::Init(int lua_entity_count, const std::vector<std::string>& preload_files)
{
	if (!btlib_link_.Init())
		return false;

	for (int i = 0; i < lua_entity_count; ++i)
	{
		EvLuaEntity* ent = new EvLuaEntity();
		if (!InitLuaEntity(ent, preload_files))
			return false;

		evlua_entity_vec_.push_back(ent);
	}

	return true;
}

bool AreaEventManager::InitLuaEntity(EvLuaEntity* lua_entity,
		                             const std::vector<std::string>& preload_files)
{
	lua_entity->Init(areaEv::area_event_func_table, areaEv::func_table_size);

	for (size_t i = 0; i < preload_files.size(); ++i)
	{
		ASSERT(lua_entity->LoadFile(preload_files[i].c_str()));
	}

	return true;
}

BTLib::BehaviorTree* AreaEventManager::CreateBTFromXML(const std::string& bt_xml_text)
{
	return bt_loader_->LoadBuffer(bt_xml_text.c_str(), bt_xml_text.length());
}

EvLuaEntity* AreaEventManager::GetLuaEntityRR()
{
	return evlua_entity_vec_[++lua_entity_rr_key_%evlua_entity_vec_.size()];
}

} // namespace gamed
