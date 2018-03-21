#include "npc_gen.h"

#include "shared/logsys/logging.h"

#include "gs/global/dbgprt.h"
#include "gs/global/game_util.h"
#include "gs/global/gmatrix.h"
#include "gs/global/randomgen.h"

#include "npc.h"


namespace gamed {

using namespace mapDataSvr;

///
/// NpcBaseSpawner
///
NpcBaseSpawner::NpcBaseSpawner(MapID wid)
	: world_id_(wid)
{
}

NpcBaseSpawner::~NpcBaseSpawner()
{
}

///
/// SpotNpcSpawner
///
SpotNpcSpawner::SpotNpcSpawner(MapID wid)
	: NpcBaseSpawner(wid)
{
}

SpotNpcSpawner::~SpotNpcSpawner()
{
}

bool SpotNpcSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const SpotMonster*> tmpvec;
	pdatamanager->QueryMapDataByType<SpotMonster>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.is_spawned = false;
		ent.pelemdata  = tmpvec[i];
		spot_npc_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}

bool SpotNpcSpawner::GenerateAllSpotNpc(NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::const_iterator it = spot_npc_map_.begin();
	for (; it != spot_npc_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		Npc* new_npc = CreateSpotNpc(it->first, it->second.pelemdata->associated_templ_id);
		if (NULL == new_npc) 
		{
			for (size_t i = 0; i < npc_vec.size(); ++i)
			{
				npc_vec[i]->Release();
				Gmatrix::FreeNpc(npc_vec[i]);
			}
			npc_vec.clear();
			return false;
		}
		else
		{
			npc_vec.push_back(new_npc);
		}
	}
	return true;
}

bool SpotNpcSpawner::CreateSpotNpc(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::iterator it = spot_npc_map_.find(elem_id);
	if (it == spot_npc_map_.end())
		return false;

	Npc* tmpnpc = CreateSpotNpc(it->second);
	if (tmpnpc == NULL)
		return false;

	npc_vec.push_back(tmpnpc);
	return true;
}

Npc* SpotNpcSpawner::CreateSpotNpc(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = spot_npc_map_.find(elem_id);
	if (it == spot_npc_map_.end())
		return NULL;

	if (tid != it->second.pelemdata->associated_templ_id)
	{
		LOG_ERROR << "SpotNpcSpawner::CreateSpotNpc() tid no matching! eid=" << elem_id;
		return NULL;
	}

	return CreateSpotNpc(it->second);
}

Npc* SpotNpcSpawner::CreateSpotNpc(const entry_t& ent)
{
	Npc* new_npc = NULL;
	new_npc = Gmatrix::AllocNpc();
	if (NULL != new_npc)
	{
		npcdef::NpcInitData init_data;
		init_data.id       = Gmatrix::AssignNpcID(new_npc);
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.templ_id = ent.pelemdata->associated_templ_id;
		init_data.dir      = ent.pelemdata->dir;
		A2DVECTOR init_pos(ent.pelemdata->coord.x, ent.pelemdata->coord.y);
		init_data.pos      = init_pos;
		if (!new_npc->Init(init_data))
		{
			new_npc->Release();
			Gmatrix::FreeNpc(new_npc);
			return NULL;
		}
	}
	return new_npc;
}

///
/// AreaMonsterSpawner
///
AreaMonsterSpawner::AreaMonsterSpawner(MapID wid)
	: NpcBaseSpawner(wid)
{
}

AreaMonsterSpawner::~AreaMonsterSpawner()
{
}

bool AreaMonsterSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	// area monster
	{
		std::vector<const AreaMonster*> tmpvec;
		pdatamanager->QueryMapDataByType<AreaMonster>(world_id_, tmpvec);
		for (size_t i = 0; i < tmpvec.size(); ++i)
		{
			// 只取明雷区域怪的数据
			if (mapDataSvr::AreaMonster::AT_VISIBLE_LANDMINE != tmpvec[i]->appear_type)
				continue;

			entry_t ent;
			ent.is_spawned   = false;
			//ent.elem_ptr_type     = entry_t::ELEM_IS_MONSTER;
			ent.pelemdata    = tmpvec[i];
			for (size_t j = 0; j < ent.pelemdata->vertexes.size(); ++j)
			{
				CGLib::Point2d point(ent.pelemdata->vertexes[j].x, ent.pelemdata->vertexes[j].y);
				ent.polygon.push_back(point);
			}
			area_monster_map_[ent.pelemdata->elem_id] = ent;
		}
	}

	return true;
}

bool AreaMonsterSpawner::GenerateAllAreaMonster(NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::const_iterator it = area_monster_map_.begin();
	for (; it != area_monster_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		bool ret = CreateAreaMonster(it->first, npc_vec);
		if (!ret)
		{
			for (size_t i = 0; i < npc_vec.size(); ++i)
			{
				npc_vec[i]->Release();
				Gmatrix::FreeNpc(npc_vec[i]);
			}
			npc_vec.clear();
			return false;
		}
	}
	return true;
}

bool AreaMonsterSpawner::CreateAreaMonster(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::const_iterator it = area_monster_map_.find(elem_id);
	if (it == area_monster_map_.end())
		return NULL;

	for (int i = 0; i < it->second.pelemdata->visible_num; ++i)
	{
		Npc* new_npc = NULL;
		new_npc      = CreateAreaMonster(it->second);
		if (NULL != new_npc)
		{
			npc_vec.push_back(new_npc);
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

Npc* AreaMonsterSpawner::CreateAreaMonster(const entry_t& ent)
{
	A2DVECTOR pos;
	if (!GeneratePos(ent.polygon, pos))
	{
		return NULL;
	}

	Npc* new_npc = NULL;
	new_npc = Gmatrix::AllocNpc();
	if (NULL != new_npc)
	{
		npcdef::NpcInitData init_data;
		init_data.id       = Gmatrix::AssignNpcID(new_npc);
		init_data.templ_id = ent.pelemdata->associated_templ_id;
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.dir      = GenerateDir();
		init_data.pos      = pos;

		// init
		if (!new_npc->Init(init_data))
		{
			new_npc->Release();
			Gmatrix::FreeNpc(new_npc);
			return NULL;
		}
	}
	return new_npc;
}

Npc* AreaMonsterSpawner::CreateAreaMonster(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = area_monster_map_.find(elem_id);
	if (it == area_monster_map_.end())
		return NULL;

	if (tid != it->second.pelemdata->associated_templ_id)
	{
		LOG_ERROR << "AreaMonsterSpawner::CreateAreaNpc() area_monster tid no matching! eid=" << elem_id;
		return NULL;
	}

	return CreateAreaMonster(it->second);
}

bool AreaMonsterSpawner::GeneratePos(const CGLib::Polygon& polygon, A2DVECTOR& dest_pos)
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

int AreaMonsterSpawner::GenerateDir()
{
	return mrand::Rand(0, 7);
}

///
/// AreaNpcSpawner
///
AreaNpcSpawner::AreaNpcSpawner(MapID wid)
	: NpcBaseSpawner(wid)
{
}

AreaNpcSpawner::~AreaNpcSpawner()
{
}

bool AreaNpcSpawner::InitGenData(mapDataSvr::MapDataManager* pdatamanager)
{
	std::vector<const AreaNpc*> tmp_vec;
	pdatamanager->QueryMapDataByType<AreaNpc>(world_id_, tmp_vec);
	for (size_t i = 0; i < tmp_vec.size(); ++i)
	{
		entry_t ent;
		ent.is_spawned = false;
		ent.pelemdata  = tmp_vec[i];
		for (size_t j = 0; j < ent.pelemdata->vertexes.size(); ++j)
		{
			CGLib::Point2d point(ent.pelemdata->vertexes[j].x, ent.pelemdata->vertexes[j].y);
			ent.polygon.push_back(point);
		}
		area_npc_map_[ent.pelemdata->elem_id] = ent;
	}

	return true;
}

bool AreaNpcSpawner::GenerateAllAreaNpc(NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::const_iterator it = area_npc_map_.begin();
	for (; it != area_npc_map_.end(); ++it)
	{
		// 不是默认激活则无视
		if (!it->second.pelemdata->is_default_activate)
			continue;

		bool ret = CreateAreaNpc(it->first, npc_vec);
		if (!ret)
		{
			for (size_t i = 0; i < npc_vec.size(); ++i)
			{
				npc_vec[i]->Release();
				Gmatrix::FreeNpc(npc_vec[i]);
			}
			npc_vec.clear();
			return false;
		}
	}
	return true;
}

bool AreaNpcSpawner::CreateAreaNpc(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec)
{
	ElemIDToEntryMap::const_iterator it = area_npc_map_.find(elem_id);
	if (it == area_npc_map_.end())
		return NULL;

	const entry_t& ent = it->second;
	for (size_t i = 0; i < ent.pelemdata->associated_templ_vec.size(); ++i)
	{
		Npc* new_npc = NULL;
		int32_t tid  = ent.pelemdata->associated_templ_vec[i];
		new_npc      = CreateAreaNpc(ent, tid);
		if (NULL != new_npc)
		{
			npc_vec.push_back(new_npc);
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

Npc* AreaNpcSpawner::CreateAreaNpc(const entry_t& ent, mapDataSvr::TemplID tid)
{
	Npc* new_npc = NULL;
	new_npc = Gmatrix::AllocNpc();
	if (NULL != new_npc)
	{
		npcdef::NpcInitData init_data;
		init_data.id       = Gmatrix::AssignNpcID(new_npc);
		init_data.templ_id = tid;
		init_data.elem_id  = ent.pelemdata->elem_id;
		init_data.dir      = 0;
		init_data.pos      = GetCentroidPos(ent.polygon);
		
		// init
		if (!new_npc->Init(init_data))
		{
			new_npc->Release();
			Gmatrix::FreeNpc(new_npc);
			return NULL;
		}
	}
	return new_npc;
}

Npc* AreaNpcSpawner::CreateAreaNpc(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid)
{
	ElemIDToEntryMap::const_iterator it = area_npc_map_.find(elem_id);
	if (it == area_npc_map_.end())
		return NULL;

	bool found = false;
	for (size_t i = 0; i < it->second.pelemdata->associated_templ_vec.size(); ++i)
	{
		if (it->second.pelemdata->associated_templ_vec[i] == tid)
		{
			found = true;
		}
	}

	// not found
	if (!found)
	{
		LOG_ERROR << "AreaNpcSpawner::CreateAreaNpc() area_npc tid not found! eid=" << elem_id;
		return NULL;
	}
	
	return CreateAreaNpc(it->second, tid);
}

A2DVECTOR AreaNpcSpawner::GetCentroidPos(const CGLib::Polygon& polygon)
{
	A2DVECTOR tmp_pos;
	polygon.centroid(tmp_pos.x, tmp_pos.y);
	return tmp_pos;
}


///
/// NpcGenerator
///
NpcGenerator::NpcGenerator(MapID wid)
	: is_inited_(false),
	  world_id_(wid),
	  spotnpc_spawner_(wid),
	  areanpc_spawner_(wid),
	  areamonster_spawner_(wid)
{
}

NpcGenerator::~NpcGenerator()
{
}

bool NpcGenerator::Init()
{
	mapDataSvr::MapDataManager* pmapdata = s_pMapData;
	if (!spotnpc_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,定点怪数据初始化失败!", world_id_);
		return false;
	}
	if (!areanpc_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,区域Npc数据初始化失败！", world_id_);
		return false;
	}
	if (!areamonster_spawner_.InitGenData(pmapdata))
	{
		__PRINTF("地图:%d,明雷区域怪数据初始化失败！", world_id_);
		return false;
	}

	is_inited_ = true;
	return true;
}

bool NpcGenerator::GenerateAllNpc(std::vector<Npc*>& npc_vec)
{
	ASSERT(is_inited_);
	if (!spotnpc_spawner_.GenerateAllSpotNpc(npc_vec))
	{
		return false;
	}
	if (!areanpc_spawner_.GenerateAllAreaNpc(npc_vec))
	{
		return false;
	}
	if (!areamonster_spawner_.GenerateAllAreaMonster(npc_vec))
	{
		return false;
	}
	return true;
}

bool NpcGenerator::IsNpcMapElem(int32_t elem_id) const
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL)
	{
		return false;
	}

	bool found = false;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_SPOT_MONSTER:
		case MAPDATA_TYPE_AREA_NPC:
			found = true;
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			{
				// 明雷
				const AreaMonster* pmon = dynamic_cast<const AreaMonster*>(pdata);
				if (pmon != NULL &&
					pmon->appear_type == AreaMonster::AT_VISIBLE_LANDMINE)
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

Npc* NpcGenerator::GenerateNpcByElemID(int32_t elem_id, int32_t templ_id)
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elem_id);
	if (!is_inited_ || pdata == NULL || pdata->map_id != world_id_)
	{
		LOG_ERROR << "找不到要生成的地图元素id:" << elem_id << " world_id:" << world_id_;
		return NULL;
	}

	Npc* new_npc = NULL;
	switch (pdata->GetType())
	{
		case MAPDATA_TYPE_SPOT_MONSTER:
			new_npc = spotnpc_spawner_.CreateSpotNpc(elem_id, templ_id);
			break;

		case MAPDATA_TYPE_AREA_NPC:
			new_npc = areanpc_spawner_.CreateAreaNpc(elem_id, templ_id);
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			new_npc = areamonster_spawner_.CreateAreaMonster(elem_id, templ_id);
			break;

		default:
			return NULL;
	}

	return new_npc;
}

bool NpcGenerator::GenerateNpcByElemID(int32_t elem_id, std::vector<Npc*>& npc_vec)
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
		case MAPDATA_TYPE_SPOT_MONSTER:
			is_success = spotnpc_spawner_.CreateSpotNpc(elem_id, npc_vec);
			break;

		case MAPDATA_TYPE_AREA_NPC:
			is_success = areanpc_spawner_.CreateAreaNpc(elem_id, npc_vec);
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			is_success = areamonster_spawner_.CreateAreaMonster(elem_id, npc_vec);
			break;

		default:
			return false;
	}

	// failure
	if (!is_success)
	{
		for (size_t i = 0; i < npc_vec.size(); ++i)
		{
			npc_vec[i]->Release();
			Gmatrix::FreeNpc(npc_vec[i]);
		}
		npc_vec.clear();
		return false;
	}

	// success
	return true;
}

bool NpcGenerator::IsAreaNpc(const Npc* pnpc) const
{
	const BaseMapData* pbase = s_pMapData->QueryBaseMapDataTempl(pnpc->elem_id());
	if (pbase != NULL)
	{
		if (pbase->GetType() == mapDataSvr::MAPDATA_TYPE_AREA_NPC)
			return true;
	}

	return false;
}

} // namespace gamed
