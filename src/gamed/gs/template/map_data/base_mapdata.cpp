#include "base_mapdata.h"

// template header
#include "spot_monster.h"
#include "area_with_rules.h"
#include "area_monster.h"
#include "area_npc.h"
#include "map_default_area.h"
#include "scene_effect.h"
#include "transfer_area.h"
#include "spot_mine.h"
#include "area_mine.h"


namespace mapDataSvr {

// init template
// 1
INIT_STAITC_MAPDATA(SpotMonster, MAPDATA_TYPE_SPOT_MONSTER);
INIT_STAITC_MAPDATA(AreaWithRules, MAPDATA_TYPE_AREA_WITH_RULES);
INIT_STAITC_MAPDATA(AreaMonster, MAPDATA_TYPE_AREA_MONSTER);
INIT_STAITC_MAPDATA(AreaNpc, MAPDATA_TYPE_AREA_NPC);

// 5
INIT_STAITC_MAPDATA(MapDefaultArea, MAPDATA_TYPE_MAP_DEFAULT_AREA);
INIT_STAITC_MAPDATA(SceneEffect, MAPDATA_TYPE_SCENE_EFFECT);
INIT_STAITC_MAPDATA(TransferArea, MAPDATA_TYPE_TRANSFER_AREA);
INIT_STAITC_MAPDATA(SpotMine, MAPDATA_TYPE_SPOT_MINE);
INIT_STAITC_MAPDATA(AreaMine, MAPDATA_TYPE_AREA_MINE);


///
/// class BaseMapData
///
void BaseMapData::Marshal()
{
	MARSHAL_MAPDATA(map_id, elem_id, is_default_activate);
	OnMarshal();
}

void BaseMapData::Unmarshal()
{
	UNMARSHAL_MAPDATA(map_id, elem_id, is_default_activate);
	OnUnmarshal();
}

bool BaseMapData::CheckDataValidity()
{
	if (map_id <= 0 || elem_id <= 0)
		return false;

	return OnCheckDataValidity();
}

///
/// class BaseMapDataManager
///
BaseMapData* BaseMapDataManager::CreatePacket(BaseMapData::Type id)
{
	return BaseMapDataManager::GetInstance()->OnCreatePacket(id);;
}

bool BaseMapDataManager::InsertPacket(uint16_t type, BaseMapData* packet)
{
	return BaseMapDataManager::GetInstance()->OnInsertPacket(type, packet);
}

bool BaseMapDataManager::IsValidType(int32_t type)
{
	if (type > MAPDATA_TYPE_INVALID && type < MAPDATA_TYPE_MAX)
	{
		return true;
	}

	return false;
}

BaseMapData* BaseMapDataManager::OnCreatePacket(BaseMapData::Type id)
{
	BaseMapDataMap::iterator it = packet_map_.find(id);
	if (packet_map_.end() == it) return NULL;

	return dynamic_cast<BaseMapData*>(it->second->Clone());
}
	
bool BaseMapDataManager::OnInsertPacket(uint16_t type, BaseMapData* packet)
{
	if (!packet_map_.insert(std::make_pair(type, packet)).second)
	{
		assert(false);
		return false;
	}

	return true;
}

} // namespace mapDataSvr
