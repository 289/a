#include "matter_gen.h"

#include "utility_lib/CGLib/cglib.h"
#include "shared/logsys/logging.h"

#include "gs/global/game_util.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"

#include "matter.h"


namespace gamed {

using namespace mapDataSvr;
using namespace matterdef;


///
/// MatterBaseSpawner
///
MatterBaseSpawner::MatterBaseSpawner(MapID wid)
	: world_id_(wid)
{
}

MatterBaseSpawner::~MatterBaseSpawner()
{
}


///
/// MapEffectSpawner
///
MapEffectSpawner::MapEffectSpawner(MapID wid)
	: MatterBaseSpawner(wid)
{
}

MapEffectSpawner::~MapEffectSpawner()
{
}

bool MapEffectSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const SceneEffect*> tmpvec;
	pdatamanager->QueryMapDataByType<SceneEffect>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.tid        = kMapEffectsTemplID;
		ent.pelemdata  = tmpvec[i];
		map_effects_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}

bool MapEffectSpawner::GenerateAllMapEffect(MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = map_effects_map_.begin();
	for (; it != map_effects_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		Matter* new_matter = CreateMapEffect(it->first, it->second.tid);
		if (NULL == new_matter) 
		{
			for (size_t i = 0; i < matter_vec.size(); ++i)
			{
				matter_vec[i]->Release();
				Gmatrix::FreeMatter(matter_vec[i]);
			}
			matter_vec.clear();
			return false;
		}
		else
		{
			matter_vec.push_back(new_matter);
		}
	}
	return true;
}

Matter* MapEffectSpawner::CreateMapEffect(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = map_effects_map_.find(elem_id);
	if (it == map_effects_map_.end())
		return NULL;

	if (it->second.tid != tid)
	{
		LOG_ERROR << "MapEffectSpawner::CreateMapEffect() map effect tid no matching! eid=" << elem_id << " tid=" << tid;
		return NULL;
	}

	return CreateMapEffect(it->second);
}

bool MapEffectSpawner::CreateMapEffect(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = map_effects_map_.find(elem_id);
	if (it == map_effects_map_.end())
		return false;

	Matter* tmpmatter = CreateMapEffect(it->second);
	if (tmpmatter == NULL)
		return false;

	matter_vec.push_back(tmpmatter);
	return true;
}

Matter* MapEffectSpawner::CreateMapEffect(const entry_t& ent)
{
	Matter* new_matter = NULL;
	new_matter = Gmatrix::AllocMatter();
	if (NULL != new_matter)
	{
		MatterInitData init_data;
		init_data.id       = Gmatrix::AssignMatterID(new_matter);
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.templ_id = ent.tid;
		A2DVECTOR init_pos(ent.pelemdata->coord.x, ent.pelemdata->coord.y);
		init_data.pos      = init_pos;
		if (!new_matter->Init(init_data))
		{
			new_matter->Release();
			Gmatrix::FreeMatter(new_matter);
			return NULL;
		}
	}
	return new_matter;
}


///
/// NpcZoneAnchorSpawner
///
NpcZoneAnchorSpawner::NpcZoneAnchorSpawner(MapID wid)
	: MatterBaseSpawner(wid)
{
}

NpcZoneAnchorSpawner::~NpcZoneAnchorSpawner()
{
}

bool NpcZoneAnchorSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const AreaNpc*> tmpvec;
	pdatamanager->QueryMapDataByType<AreaNpc>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.pelemdata  = tmpvec[i];
		ent.tid        = kMapNpcZoneAnchorTemplID;
		CGLib::Polygon polygon;
		for (size_t j = 0; j < ent.pelemdata->vertexes.size(); ++j)
		{
			CGLib::Point2d point(ent.pelemdata->vertexes[j].x, ent.pelemdata->vertexes[j].y);
			polygon.push_back(point);
		}
		ent.anchor_pos = GetCentroidPos(polygon);
		npc_zone_anchor_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}

bool NpcZoneAnchorSpawner::GenerateAllNpcZoneAnchor(MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = npc_zone_anchor_map_.begin();
	for (; it != npc_zone_anchor_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		Matter* new_matter = CreateNpcZoneAnchor(it->first, it->second.tid);
		if (NULL == new_matter) 
		{
			for (size_t i = 0; i < matter_vec.size(); ++i)
			{
				matter_vec[i]->Release();
				Gmatrix::FreeMatter(matter_vec[i]);
			}
			matter_vec.clear();
			return false;
		}
		else
		{
			matter_vec.push_back(new_matter);
		}
	}
	return true;
}

Matter* NpcZoneAnchorSpawner::CreateNpcZoneAnchor(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = npc_zone_anchor_map_.find(elem_id);
	if (it == npc_zone_anchor_map_.end())
		return NULL;

	if (it->second.tid != tid)
	{
		LOG_ERROR << "NpcZoneAnchorSpawner::CreateNpcZoneAnchor() npc zone anchor tid no matching! eid=" << elem_id << " tid=" << tid;
		return NULL;
	}

	return CreateNpcZoneAnchor(it->second);
}

bool NpcZoneAnchorSpawner::CreateNpcZoneAnchor(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = npc_zone_anchor_map_.find(elem_id);
	if (it == npc_zone_anchor_map_.end())
		return false;

	Matter* tmpmatter = CreateNpcZoneAnchor(it->second);
	if (tmpmatter == NULL)
		return false;

	matter_vec.push_back(tmpmatter);
	return true;
}

Matter* NpcZoneAnchorSpawner::CreateNpcZoneAnchor(const entry_t& ent)
{
	Matter* new_matter = NULL;
	new_matter = Gmatrix::AllocMatter();
	if (NULL != new_matter)
	{
		MatterInitData init_data;
		init_data.id       = Gmatrix::AssignMatterID(new_matter);
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.templ_id = ent.tid;
		A2DVECTOR init_pos(ent.anchor_pos.x, ent.anchor_pos.y);
		init_data.pos      = init_pos;
		if (!new_matter->Init(init_data))
		{
			new_matter->Release();
			Gmatrix::FreeMatter(new_matter);
			return NULL;
		}
	}
	return new_matter;
}

A2DVECTOR NpcZoneAnchorSpawner::GetCentroidPos(const CGLib::Polygon& polygon)
{
	A2DVECTOR tmp_pos;
	polygon.centroid(tmp_pos.x, tmp_pos.y);
	return tmp_pos;
}


///
/// SpotMineSpawner
///
SpotMineSpawner::SpotMineSpawner(MapID wid)
	: MatterBaseSpawner(wid)
{
}

SpotMineSpawner::~SpotMineSpawner()
{
}

bool SpotMineSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const SpotMine*> tmpvec;
	pdatamanager->QueryMapDataByType<SpotMine>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.pelemdata  = tmpvec[i];
		spot_mine_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}

bool SpotMineSpawner::GenerateAllSpotMine(MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = spot_mine_map_.begin();
	for (; it != spot_mine_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		Matter* new_matter = CreateSpotMine(it->first, it->second.pelemdata->associated_templ_id);
		if (NULL == new_matter) 
		{
			for (size_t i = 0; i < matter_vec.size(); ++i)
			{
				matter_vec[i]->Release();
				Gmatrix::FreeMatter(matter_vec[i]);
			}
			matter_vec.clear();
			return false;
		}
		else
		{
			matter_vec.push_back(new_matter);
		}
	}
	return true;
}

Matter* SpotMineSpawner::CreateSpotMine(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = spot_mine_map_.find(elem_id);
	if (it == spot_mine_map_.end())
		return NULL;

	if (it->second.pelemdata->associated_templ_id != tid)
	{
		LOG_ERROR << "SpotMineSpawner::CreateSpotMine() spot mine tid no matching! eid=" << elem_id << " tid=" << tid;
		return NULL;
	}

	return CreateSpotMine(it->second);
}

bool SpotMineSpawner::CreateSpotMine(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = spot_mine_map_.find(elem_id);
	if (it == spot_mine_map_.end())
		return false;

	Matter* tmpmatter = CreateSpotMine(it->second);
	if (tmpmatter == NULL)
		return false;

	matter_vec.push_back(tmpmatter);
	return true;
}

Matter* SpotMineSpawner::CreateSpotMine(const entry_t& ent)
{
	Matter* new_matter = NULL;
	new_matter = Gmatrix::AllocMatter();
	if (NULL != new_matter)
	{
		MatterInitData init_data;
		init_data.id       = Gmatrix::AssignMatterID(new_matter);
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.templ_id = ent.pelemdata->associated_templ_id;
		A2DVECTOR init_pos(ent.pelemdata->coord.x, ent.pelemdata->coord.y);
		init_data.pos      = init_pos;
		if (!new_matter->Init(init_data))
		{
			new_matter->Release();
			Gmatrix::FreeMatter(new_matter);
			return NULL;
		}
	}
	return new_matter;
}


///
/// AreaMineSpawner
///
AreaMineSpawner::AreaMineSpawner(MapID wid)
	: MatterBaseSpawner(wid)
{
}

AreaMineSpawner::~AreaMineSpawner()
{
}

bool AreaMineSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const AreaMine*> tmpvec;
	pdatamanager->QueryMapDataByType<AreaMine>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.pelemdata  = tmpvec[i];
		for (size_t j = 0; j < ent.pelemdata->vertexes.size(); ++j)
		{
			CGLib::Point2d point(ent.pelemdata->vertexes[j].x, ent.pelemdata->vertexes[j].y);
			ent.polygon.push_back(point);
		}
		area_mine_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}

bool AreaMineSpawner::GenerateAllAreaMine(MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = area_mine_map_.begin();
	for (; it != area_mine_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		bool ret = CreateAreaMine(it->first, matter_vec);
		if (!ret)
		{
			for (size_t i = 0; i < matter_vec.size(); ++i)
			{
				matter_vec[i]->Release();
				Gmatrix::FreeMatter(matter_vec[i]);
			}
			matter_vec.clear();
			return false;
		}
	}

	return true;
}

bool AreaMineSpawner::CreateAreaMine(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec)
{
	ElemIDToEntryMap::const_iterator it = area_mine_map_.find(elem_id);
	if (it == area_mine_map_.end())
		return NULL;

	for (int i = 0; i < (int)it->second.pelemdata->mine_gen_num; ++i)
	{
		Matter* new_matter = NULL;
		new_matter         = CreateAreaMine(it->second);
		if (NULL != new_matter)
		{
			matter_vec.push_back(new_matter);
		}
		/*
		else
		{
			return false;
		}
		*/
	}

	return true;
}

Matter* AreaMineSpawner::CreateAreaMine(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = area_mine_map_.find(elem_id);
	if (it == area_mine_map_.end())
		return NULL;

	if (it->second.pelemdata->associated_templ_id != tid)
	{
		LOG_ERROR << "AreaMineSpawner::CreateAreaMine() area mine tid no matching! eid=" << elem_id << " tid=" << tid;
		return NULL;
	}

	return CreateAreaMine(it->second);
}

Matter* AreaMineSpawner::CreateAreaMine(const entry_t& ent)
{
	A2DVECTOR pos;
	if (!GeneratePos(ent.polygon, pos))
	{
		return NULL;
	}

	Matter* new_matter = NULL;
	new_matter = Gmatrix::AllocMatter();
	if (NULL != new_matter)
	{
		MatterInitData init_data;
		init_data.id       = Gmatrix::AssignMatterID(new_matter);
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.templ_id = ent.pelemdata->associated_templ_id;
		init_data.pos      = pos;

		// init
		if (!new_matter->Init(init_data))
		{
			new_matter->Release();
			Gmatrix::FreeMatter(new_matter);
			return NULL;
		}
	}
	return new_matter;
}

bool AreaMineSpawner::GeneratePos(const CGLib::Polygon& polygon, A2DVECTOR& dest_pos)
{
    A2DVECTOR pos;
	pos.x         = polygon.get_point(0).x();
	pos.y         = polygon.get_point(0).y();

	bool found    = false;
	int max_size  = polygon.size();
	int countdown = 64; // 最多随64次
	while (countdown > 0)
    {
        int array[4] = {0};
        int step = mrand::Rand(1, max_size - 1);
        array[0] = mrand::Rand(0, max_size - 1);
        array[1] = (array[0] + 1*step) % max_size;
        array[2] = (array[0] + 2*step) % max_size;
        array[3] = (array[0] + 3*step) % max_size;
        std::sort(array, array+4);

        CGLib::Point2d tmp_pt = CGLib::generate_random_point(polygon.get_point(array[0]), 
                                                             polygon.get_point(array[1]), 
                                                             polygon.get_point(array[2]),
                                                             polygon.get_point(array[3]));
        // 随出的是否是有效点
        if (polygon.point_in_polygon(tmp_pt))
        {
            pos.x = tmp_pt.x();
            pos.y = tmp_pt.y();

            // 位置需要是一个可达点
            if (pathfinder::IsWalkable(world_id_, A2D_TO_PF(pos)))
            {
                found = true;
                break;
            }
        }

        --countdown;
    }

    if (found)
    {
        dest_pos = pos;
    }
    else
    {
        LOG_ERROR << "GeneratePos failure!";
    }
	return found;
}


///
/// MatterGenerator
///
MatterGenerator::MatterGenerator(MapID wid)
	: is_inited_(false),
	  world_id_(wid),
	  mapeffect_spawner_(wid),
	  npczone_anchor_spawner_(wid),
	  spot_mine_spawner_(wid),
	  area_mine_spawner_(wid)
{
}
	
MatterGenerator::~MatterGenerator()
{
	is_inited_ = false;
}

bool MatterGenerator::Init()
{
	mapDataSvr::MapDataManager* pmapdata = s_pMapData;
	if (!mapeffect_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,地图光效matter数据初始化失败!", world_id_);
		return false;
	}

	if (!npczone_anchor_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,npc区域锚点数据初始化失败！", world_id_);
		return false;
	}

	if (!spot_mine_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,定点矿数据初始化失败！", world_id_);
		return false;
	}

	if (!area_mine_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,区域矿数据初始化失败！", world_id_);
		return false;
	}

	is_inited_ = true;
	return true;
}

bool MatterGenerator::GenerateAllMatter(std::vector<Matter*>& matter_vec)
{
	ASSERT(is_inited_);
	if (!mapeffect_spawner_.GenerateAllMapEffect(matter_vec))
	{
		return false;
	}

	if (!npczone_anchor_spawner_.GenerateAllNpcZoneAnchor(matter_vec))
	{
		return false;
	}

	if (!spot_mine_spawner_.GenerateAllSpotMine(matter_vec))
	{
		return false;
	}

	if (!area_mine_spawner_.GenerateAllAreaMine(matter_vec))
	{
		return false;
	}

	return true;
}

bool MatterGenerator::IsMatterMapElem(int32_t elem_id) const
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL)
	{
		return false;
	}

	bool found = false;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_SCENE_EFFECT:
		case MAPDATA_TYPE_AREA_NPC:
		case MAPDATA_TYPE_SPOT_MINE:
		case MAPDATA_TYPE_AREA_MINE:
			found = true;
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

Matter* MatterGenerator::GenerateMatterByElemID(int32_t elem_id, int32_t templ_id)
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL || pdata->map_id != world_id_)
	{
		LOG_ERROR << "找不到要生成的地图元素id:" << elem_id << " world_id:" << world_id_;
		return NULL;
	}

	Matter* new_matter = NULL;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_SCENE_EFFECT:
			new_matter = mapeffect_spawner_.CreateMapEffect(elem_id, templ_id);
			break;

		case MAPDATA_TYPE_AREA_NPC:
			new_matter = npczone_anchor_spawner_.CreateNpcZoneAnchor(elem_id, templ_id);
			break;

		case MAPDATA_TYPE_SPOT_MINE:
			new_matter = spot_mine_spawner_.CreateSpotMine(elem_id, templ_id);
			break;

		case MAPDATA_TYPE_AREA_MINE:
			new_matter = area_mine_spawner_.CreateAreaMine(elem_id, templ_id);
			break;

		default:
			return NULL;
	}

	return new_matter;
}

bool MatterGenerator::GenerateMatterByElemID(int32_t elem_id, std::vector<Matter*>& matter_vec)
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL || pdata->map_id != world_id_)
	{
		LOG_ERROR << "找不到要生成的地图元素id:" << elem_id << " world_id:" << world_id_;
		return false;
	}

	bool is_success = false;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_SCENE_EFFECT:
			is_success = mapeffect_spawner_.CreateMapEffect(elem_id, matter_vec);
			break;

		case MAPDATA_TYPE_AREA_NPC:
			is_success = npczone_anchor_spawner_.CreateNpcZoneAnchor(elem_id, matter_vec);
			break;

		case MAPDATA_TYPE_SPOT_MINE:
			is_success = spot_mine_spawner_.CreateSpotMine(elem_id, matter_vec);
			break;

		case MAPDATA_TYPE_AREA_MINE:
			is_success = area_mine_spawner_.CreateAreaMine(elem_id, matter_vec);
			break;

		default:
			return false;
	}

	// failure
	if (!is_success)
	{
		for (size_t i = 0; i < matter_vec.size(); ++i)
		{
			matter_vec[i]->Release();
			Gmatrix::FreeMatter(matter_vec[i]);
		}
		matter_vec.clear();
		return false;
	}

	// success
	return true;
}

} // namespace gamed
