#ifndef GAMED_GS_OBJ_MATTER_GEN_H_
#define GAMED_GS_OBJ_MATTER_GEN_H_

#include "utility_lib/CGLib/cglib.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/math_types.h"
#include "gs/global/game_types.h"


namespace CGLib {
	class Polygon;
} // namespace CGLib


namespace gamed {

class Matter;

/**
 * @brief MatterBaseSpawner
 */
class MatterBaseSpawner
{
public:
	MatterBaseSpawner(MapID wid);
	virtual ~MatterBaseSpawner();

protected:
	typedef std::vector<Matter*>& MatterVectorRef;

protected:
	const MapID   world_id_;
};

/**
 * @brief 地图光效（效果）
 */
class MapEffectSpawner : public MatterBaseSpawner
{
	struct entry_t
	{
		entry_t()
			: tid(0),
			  pelemdata(NULL)
		{ }

		mapDataSvr::TemplID tid;
		const mapDataSvr::SceneEffect* pelemdata;
	};

public:
	MapEffectSpawner(MapID wid);
	virtual ~MapEffectSpawner();

	bool     InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool     GenerateAllMapEffect(MatterVectorRef matter_vec);
	Matter*  CreateMapEffect(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool     CreateMapEffect(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec);


private:
	Matter*  CreateMapEffect(const entry_t& ent);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    map_effects_map_;
};


/**
 * @brief Npc区域的锚点
 *    （1）这个类只是一个锚点，用于在AOI里通知客户端该npc区域激活。
 */
class NpcZoneAnchorSpawner : public MatterBaseSpawner
{
	struct entry_t
	{
		entry_t()
			: tid(0),
			  pelemdata(NULL)
		{ }

		mapDataSvr::TemplID tid;
		A2DVECTOR anchor_pos;
		const mapDataSvr::AreaNpc* pelemdata;
	};

public:
	NpcZoneAnchorSpawner(MapID wid);
	virtual ~NpcZoneAnchorSpawner();

	bool      InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool      GenerateAllNpcZoneAnchor(MatterVectorRef matter_vec);
	Matter*   CreateNpcZoneAnchor(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool      CreateNpcZoneAnchor(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec);


private:
	Matter*   CreateNpcZoneAnchor(const entry_t& ent);
	A2DVECTOR GetCentroidPos(const CGLib::Polygon& polygon);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    npc_zone_anchor_map_;
};


/**
 * @brief SpotMineSpawner
 */
class SpotMineSpawner : public MatterBaseSpawner
{
	struct entry_t
	{
		entry_t() : pelemdata(NULL) { }
		const mapDataSvr::SpotMine* pelemdata;
	};

public:
	SpotMineSpawner(MapID wid);
	virtual ~SpotMineSpawner();

	bool      InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool      GenerateAllSpotMine(MatterVectorRef matter_vec);
	Matter*   CreateSpotMine(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool      CreateSpotMine(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec);


private:
	Matter*   CreateSpotMine(const entry_t& ent);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    spot_mine_map_;
};


/**
 * @brief AreaMineSpawner
 */
class AreaMineSpawner : public MatterBaseSpawner
{
	struct entry_t
	{
		entry_t() : pelemdata(NULL) { }
		CGLib::Polygon  polygon;
		const mapDataSvr::AreaMine* pelemdata;
	};

public:
	AreaMineSpawner(MapID wid);
	virtual ~AreaMineSpawner();

	bool      InitGenData(mapDataSvr::MapDataManager* pdatamanager);
	bool      GenerateAllAreaMine(MatterVectorRef matter_vec);
	Matter*   CreateAreaMine(mapDataSvr::ElemID elem_id, mapDataSvr::TemplID tid);
	bool      CreateAreaMine(mapDataSvr::ElemID elem_id, MatterVectorRef matter_vec);


private:
	Matter*   CreateAreaMine(const entry_t& ent);
	bool      GeneratePos(const CGLib::Polygon& polygon, A2DVECTOR& dest_pos);


private:
	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap    area_mine_map_;
};


/**
 * @brief MatterGenerator
 */
class MatterGenerator
{
public:
	MatterGenerator(MapID wid);
	~MatterGenerator();

	bool    Init();
	bool    GenerateAllMatter(std::vector<Matter*>& matter_vec);
	Matter* GenerateMatterByElemID(int32_t elem_id, int32_t templ_id);
	bool    GenerateMatterByElemID(int32_t elem_id, std::vector<Matter*>& matter_vec);
	bool    IsMatterMapElem(int32_t elem_id) const;


private:
	bool                 is_inited_;
	const MapID          world_id_;
	MapEffectSpawner     mapeffect_spawner_;
	NpcZoneAnchorSpawner npczone_anchor_spawner_;
	SpotMineSpawner      spot_mine_spawner_;
	AreaMineSpawner      area_mine_spawner_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_MATTER_GEN_H_
