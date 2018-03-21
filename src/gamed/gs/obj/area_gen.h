#ifndef GAMED_GS_OBJ_AREA_GEN_H_
#define GAMED_GS_OBJ_AREA_GEN_H_

#include "utility_lib/CGLib/cglib.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/game_types.h"


namespace gamed {

class AreaObj;

/**
 * @brief AreaBaseSpawner
 */
class AreaBaseSpawner
{
public:
	AreaBaseSpawner(MapID wid);
	virtual ~AreaBaseSpawner();
	
	virtual AreaObj* CreateAreaObj(mapDataSvr::ElemID elem_id) { return NULL; }
	virtual bool     InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman) { return false; }

	bool    GenerateAllAreas(std::vector<AreaObj*>& area_vec);


protected:
	struct entry_t
	{
		entry_t()
			: is_spawned(false),
			  pelemdata(NULL)
		{ }

		bool      is_spawned;
		A2DVECTOR centroid_pos;
		const mapDataSvr::BaseMapData* pelemdata;
	};

	AreaObj* CreateSpecAreaObj(const entry_t& ent);
	template <typename T>
	bool     InitGenData(mapDataSvr::MapDataManager* pmapdataman);
	A2DVECTOR GetCentroidPos(const CGLib::Polygon& polygon);


protected:
	const MapID    world_id_;

	typedef std::map<mapDataSvr::ElemID, entry_t> ElemIDToEntryMap;
	ElemIDToEntryMap areas_map_;
};

///
/// template func
///
template<typename T>
bool AreaBaseSpawner::InitGenData(mapDataSvr::MapDataManager* pmapdataman)
{
	std::vector<const T*> tmpvec;
	pmapdataman->QueryMapDataByType<T>(world_id_, tmpvec);
	for (size_t i = 0; i < tmpvec.size(); ++i)
	{
		entry_t ent;
		ent.is_spawned = false;
		ent.pelemdata  = static_cast<const mapDataSvr::BaseMapData*>(tmpvec[i]);

		CGLib::Polygon polygon;
		const T* elem_subclass = dynamic_cast<const T*>(ent.pelemdata);
		for (size_t j = 0; j < elem_subclass->vertexes.size(); ++j)
		{
			CGLib::Point2d point(elem_subclass->vertexes[j].x, elem_subclass->vertexes[j].y);
			polygon.push_back(point);
		}
		ent.centroid_pos = GetCentroidPos(polygon);

		areas_map_[ent.pelemdata->elem_id] = ent;
	}
	return true;
}


/**
 * @brief AreaWithRulesSpawner
 */
class AreaWithRulesSpawner : public AreaBaseSpawner
{
public:
	AreaWithRulesSpawner(MapID wid);
	virtual ~AreaWithRulesSpawner();

	virtual AreaObj* CreateAreaObj(mapDataSvr::ElemID elem_id);
	virtual bool     InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman);


private:
};


/**
 * @brief TransferAreaSpawner
 */
class TransferAreaSpawner : public AreaBaseSpawner
{
public:
	TransferAreaSpawner(MapID wid);
	virtual ~TransferAreaSpawner();

	virtual AreaObj* CreateAreaObj(mapDataSvr::ElemID elem_id);
	virtual bool     InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman);


private:
};


/**
 * @brief LandmineAreaSpawner
 */
class LandmineAreaSpawner : public AreaBaseSpawner
{
public:
	LandmineAreaSpawner(MapID wid);
	virtual ~LandmineAreaSpawner();

	virtual AreaObj* CreateAreaObj(mapDataSvr::ElemID elem_id);
	virtual bool     InitAreaGenData(mapDataSvr::MapDataManager* pmapdataman);

private:
};


/**
 * @brief AreaGenerator
 *    1.一张地图只有一个AreaGenerator
 */
class AreaGenerator
{
public:
	AreaGenerator(MapID wid);
	~AreaGenerator();

	bool     Init();
	bool     GenerateAllAreas(std::vector<AreaObj*>& area_vec);
	AreaObj* GenerateAreaObjByElemID(int32_t elem_id);
	bool     IsAreaMapElem(int32_t elem_id) const;


private:
	bool                 is_inited_;
	const MapID          world_id_;
	AreaWithRulesSpawner rules_area_sp_;
	TransferAreaSpawner  transfer_area_sp_;
	LandmineAreaSpawner  landmine_area_sp_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_AREA_GEN_H_
