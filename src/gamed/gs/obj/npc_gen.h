#ifndef GAMED_GS_OBJ_NPC_GEN_H_
#define GAMED_GS_OBJ_NPC_GEN_H_

#include "utility_lib/CGLib/cglib.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/math_types.h"
#include "gs/global/game_types.h"


namespace gamed {

class Npc;

/**
 * @brief NpcBaseSpawner
 */
class NpcBaseSpawner
{
public:
	NpcBaseSpawner(MapID wid);
	virtual ~NpcBaseSpawner();

protected:
	typedef std::vector<Npc*>& NpcVectorRef;

protected:
	const MapID    world_id_;
};


/**
 * @brief SpotNpcSpawner
 */
class SpotNpcSpawner : public NpcBaseSpawner
{
	struct entry_t
	{
		entry_t()
			: is_spawned(false),
			  pelemdata(NULL)
		{ }

		bool is_spawned;
		const mapDataSvr::SpotMonster* pelemdata;
	};

public:
	SpotNpcSpawner(MapID wid);
	virtual ~SpotNpcSpawner();

	bool    InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool    GenerateAllSpotNpc(NpcVectorRef npc_vec);
	bool    CreateSpotNpc(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec);
	Npc*    CreateSpotNpc(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);


private:
	Npc*    CreateSpotNpc(const entry_t& ent);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    spot_npc_map_;
};


/**
 * @brief AreaMonsterSpawner
 */
class AreaMonsterSpawner : public NpcBaseSpawner
{
	struct entry_t
	{
		entry_t()
			: is_spawned(false),
			  pelemdata(NULL)
		{ }

		bool            is_spawned;
		CGLib::Polygon  polygon;
		const mapDataSvr::AreaMonster* pelemdata;
	};

public:
	AreaMonsterSpawner(MapID wid);
	virtual ~AreaMonsterSpawner();

	bool    InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool    GenerateAllAreaMonster(NpcVectorRef npc_vec);
	Npc*    CreateAreaMonster(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool    CreateAreaMonster(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec);


private:
	Npc*    CreateAreaMonster(const entry_t& ent);
	int     GenerateDir();
	bool    GeneratePos(const CGLib::Polygon& polygon, A2DVECTOR& dest_pos);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    area_monster_map_;
};


/**
 * @brief AreaNpcSpawner
 */
class AreaNpcSpawner : public NpcBaseSpawner
{
	struct entry_t
	{
		entry_t()
			: is_spawned(false),
			  pelemdata(NULL)
		{ }
				
		bool            is_spawned;
		CGLib::Polygon  polygon;
		const mapDataSvr::AreaNpc* pelemdata;
	};

public:
	AreaNpcSpawner(MapID wid);
	virtual ~AreaNpcSpawner();

	bool    InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool    GenerateAllAreaNpc(NpcVectorRef npc_vec);
	Npc*    CreateAreaNpc(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool    CreateAreaNpc(mapDataSvr::ElemID elem_id, NpcVectorRef npc_vec);


private:
	Npc*      CreateAreaNpc(const entry_t& ent, mapDataSvr::TemplID tid);
	A2DVECTOR GetCentroidPos(const CGLib::Polygon& polygon);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    area_npc_map_;
};


/**
 * @brief NpcGenerator
 *    1.一张地图只有一个NpcGenerator
 */
class NpcGenerator
{
public:
	NpcGenerator(MapID wid);
	~NpcGenerator();

	bool    Init();
	bool    GenerateAllNpc(std::vector<Npc*>& npc_vec);
	Npc*    GenerateNpcByElemID(int32_t elem_id, int32_t templ_id);
	bool    GenerateNpcByElemID(int32_t elem_id, std::vector<Npc*>& npc_vec);
	bool    IsNpcMapElem(int32_t elem_id) const;
	bool    IsAreaNpc(const Npc* pnpc) const;


private:
	bool               is_inited_;
	const MapID        world_id_;
	SpotNpcSpawner     spotnpc_spawner_;
	AreaNpcSpawner     areanpc_spawner_;
	AreaMonsterSpawner areamonster_spawner_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_NPC_GEN_H_
