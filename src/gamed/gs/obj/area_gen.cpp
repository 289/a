#include "area_gen.h"

#include "shared/logsys/logging.h"
#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"

#include "area.h"


namespace gamed {

using namespace mapDataSvr;

///
/// AreaBaseSpawner
///
AreaBaseSpawner::AreaBaseSpawner(MapID wid)
	: world_id_(wid)
{
}

AreaBaseSpawner::~AreaBaseSpawner()
{
}

bool AreaBaseSpawner::GenerateAllAreas(std::vector<AreaObj*>& area_vec)
{
	ElemIDToEntryMap::const_iterator it = areas_map_.begin();
	for (; it != areas_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		AreaObj* new_area = CreateAreaObj(it->first);
		if (NULL == new_area) 
		{
			for (size_t i = 0; i < area_vec.size(); ++i)
			{
				area_vec[i]->Release();
				Gmatrix::FreeAreaObj(area_vec[i]);
			}
			area_vec.clear();
			return false;
		}
		else
		{
			area_vec.push_back(new_area);
		}
	}
	return true;
}

AreaObj* AreaBaseSpawner::CreateSpecAreaObj(const entry_t& ent)
{
	AreaObj* new_area = NULL;
	new_area = Gmatrix::AllocAreaObj();
	if (NULL != new_area)
	{
		AreaInitData init_data;
		init_data.id       = Gmatrix::AssignAreaObjID(new_area);
		init_data.pos      = ent.centroid_pos;
		init_data.elem_id  = ent.pelemdata->elem_id;
		if (!new_area->Init(init_data))
		{
			new_area->Release();
			Gmatrix::FreeAreaObj(new_area);
			return NULL;
		}
	}
	return new_area;
}

A2DVECTOR AreaBaseSpawner::GetCentroidPos(const CGLib::Polygon& polygon)
{
	A2DVECTOR tmp_pos;
	polygon.centroid(tmp_pos.x, tmp_pos.y);
	return tmp_pos;
}


///
/// AreaWithRulesSpawner
///
AreaWithRulesSpawner::AreaWithRulesSpawner(MapID wid)
	: AreaBaseSpawner(wid)
{
}

AreaWithRulesSpawner::~AreaWithRulesSpawner()
{
}

AreaObj* AreaWithRulesSpawner::CreateAreaObj(mapDataSvr::ElemID elem_id)
{
	ElemIDToEntryMap::const_iterator it = areas_map_.find(elem_id);
	if (it == areas_map_.end())
		return NULL;

	return CreateSpecAreaObj(it->second);
}

bool AreaWithRulesSpawner::InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman)
{
	return InitGenData<mapDataSvr::AreaWithRules>(pmapdataman);
}


///
/// TransferAreaSpawner
///
TransferAreaSpawner::TransferAreaSpawner(MapID wid)
	: AreaBaseSpawner(wid)
{
}

TransferAreaSpawner::~TransferAreaSpawner()
{
}

AreaObj* TransferAreaSpawner::CreateAreaObj(mapDataSvr::ElemID elem_id)
{
	ElemIDToEntryMap::const_iterator it = areas_map_.find(elem_id);
	if (it == areas_map_.end())
		return NULL;

	return CreateSpecAreaObj(it->second);
}

bool TransferAreaSpawner::InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman)
{
	return InitGenData<mapDataSvr::TransferArea>(pmapdataman);
}


///
/// LandmineAreaSpawner
/// 
LandmineAreaSpawner::LandmineAreaSpawner(MapID wid)
	: AreaBaseSpawner(wid)
{
}

LandmineAreaSpawner::~LandmineAreaSpawner()
{
}

AreaObj* LandmineAreaSpawner::CreateAreaObj(mapDataSvr::ElemID elem_id)
{
	ElemIDToEntryMap::const_iterator it = areas_map_.find(elem_id);
	if (it == areas_map_.end())
		return NULL;

	return CreateSpecAreaObj(it->second);
}

bool LandmineAreaSpawner::InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman)
{
	if (!InitGenData<mapDataSvr::AreaMonster>(pmapdataman))
	{
		return false;
	}

	ElemIDToEntryMap::iterator it = areas_map_.begin();
	while (it != areas_map_.end())
	{
		const mapDataSvr::AreaMonster* pelem = dynamic_cast<const AreaMonster*>(it->second.pelemdata);
		if (mapDataSvr::AreaMonster::AT_LANDMINE != pelem->appear_type)
		{
			areas_map_.erase(it++);
		}
		else
		{
			++it;
		}
	}

	return true;
}


///
/// AreaGenerator
///
AreaGenerator::AreaGenerator(MapID wid)
	: is_inited_(false),
	  world_id_(wid),
	  rules_area_sp_(wid), 
	  transfer_area_sp_(wid),
	  landmine_area_sp_(wid)	
{
}

AreaGenerator::~AreaGenerator()
{
}

bool AreaGenerator::Init()
{
	mapDataSvr::MapDataManager* pmapdata = s_pMapData;
	if (!rules_area_sp_.InitAreaGenData(pmapdata))
	{
		__PRINTF("地图:%d,规则区域数据初始化失败!", world_id_);
		return false;
	}
	
	if (!transfer_area_sp_.InitAreaGenData(pmapdata))
	{
		__PRINTF("地图:%d,传送区域数据初始化失败!", world_id_);
		return false;
	}

	if (!landmine_area_sp_.InitAreaGenData(pmapdata))
	{
		__PRINTF("地图:%d,暗雷区域数据初始化失败!", world_id_);
		return false;
	}

	is_inited_ = true;
	return true;
}

bool AreaGenerator::GenerateAllAreas(std::vector<AreaObj*>& area_vec)
{
	ASSERT(is_inited_);
	if (!rules_area_sp_.GenerateAllAreas(area_vec))
	{
		return false;
	}
	if (!transfer_area_sp_.GenerateAllAreas(area_vec))
	{
		return false;
	}
	if (!landmine_area_sp_.GenerateAllAreas(area_vec))
	{
		return false;
	}
	return true;
}

bool AreaGenerator::IsAreaMapElem(int32_t elem_id) const
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL)
	{
		return false;
	}

	bool found = false;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_AREA_WITH_RULES:
		case MAPDATA_TYPE_TRANSFER_AREA:
			found = true;
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			{
				// 暗雷
				const AreaMonster* pmon = dynamic_cast<const AreaMonster*>(pdata);
				if (pmon != NULL &&
					pmon->appear_type == AreaMonster::AT_LANDMINE)
				{
					found = true;
				}
			}
			break;

		default:
			found = false;
			break;
	}

	if (pdata->map_id != world_id_)
	{
		LOG_ERROR << "不是本地图的地图元素！eid:" << elem_id << " world_id:" << world_id_;
		return false;
	}

	return found;
}

AreaObj* AreaGenerator::GenerateAreaObjByElemID(int32_t elem_id)
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL || pdata->map_id != world_id_)
	{
		LOG_ERROR << "找不到要生成的地图元素id:" << elem_id << " world_id:" << world_id_;
		return NULL;
	}

	AreaObj* new_areaobj = NULL;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_AREA_WITH_RULES:
			new_areaobj = rules_area_sp_.CreateAreaObj(elem_id);
			break;

		case MAPDATA_TYPE_TRANSFER_AREA:
			new_areaobj = transfer_area_sp_.CreateAreaObj(elem_id);
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			new_areaobj = landmine_area_sp_.CreateAreaObj(elem_id);
			break;

		default:
			return NULL;
	}

	return new_areaobj;
}

} // namespace gamed
