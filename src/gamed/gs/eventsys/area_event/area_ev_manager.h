#ifndef GAMED_GS_EVENT_WORLD_EVENT_MANAGER_H_
#define GAMED_GS_EVENT_WORLD_EVENT_MANAGER_H_

#include "gs/eventsys/src/event_sys.h"

#include "area_btlib_link.h"


namespace gamed {

class EvLuaEntity;

///
/// AreaEventManager
///
class AreaEventManager
{
public:
	AreaEventManager();
	~AreaEventManager();

	bool Init(int lua_entity_count, const std::vector<std::string>& preload_files);
	BTLib::BehaviorTree* CreateBTFromXML(const std::string& bt_xml_text);
	EvLuaEntity* GetLuaEntityRR();


private:
	bool InitLuaEntity(EvLuaEntity* lua_entity,
			           const std::vector<std::string>& preload_files);

private:
	int32_t    lua_entity_rr_key_;
	AreaEvLibraryLink          btlib_link_;
	BTLib::LoadBehaviorTree*   bt_loader_;
	std::vector<EvLuaEntity*>  evlua_entity_vec_;
};

} // namespace gamed

#endif // GAMED_GS_EVENT_WORLD_EVENT_MANAGER_H_
