#ifndef GAMED_GS_TEMPLATE_MAPDATA_MAPDATA_MANAGER_H_
#define GAMED_GS_TEMPLATE_MAPDATA_MAPDATA_MANAGER_H_

#include <map>

#include "map_types.h"
#include "base_mapdata.h"

// template header
#include "spot_monster.h"
#include "area_monster.h"
#include "area_npc.h"
#include "area_with_rules.h"
#include "map_default_area.h"
#include "scene_effect.h"
#include "transfer_area.h"
#include "spot_mine.h"
#include "area_mine.h"


namespace shared {
namespace net {

	class Buffer;
	template <typename T>
	class TemplPacketCodec;

} // namespace net
} // namespace shared


namespace mapDataSvr {

class MapDataManager : public shared::Singleton<MapDataManager>
{
	friend class shared::Singleton<MapDataManager>;
public:
	static inline MapDataManager* GetInstance() {
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool WriteToFile(const char* path, std::vector<BaseMapData*>& vec_mapdata);

	template<class T> void QueryMapDataByType(MapID mapid, std::vector<const T*>& ptr_vec);
	template<class T> const T* QueryMapDataTempl(ElemID id);
	const BaseMapData* QueryBaseMapDataTempl(ElemID id);


protected:
	MapDataManager();
	~MapDataManager();

	void InsertToMapByMapID(BaseMapData* pelem);


private:
	shared::net::TemplPacketCodec<BaseMapData>* codec_;

	typedef std::vector<const BaseMapData*> BaseMapDataVec;
	typedef std::map<ElemID, const BaseMapData*> IdToMapDataMap;
	IdToMapDataMap      id_query_map_;

	typedef int16_t PacketType;
	typedef std::map<PacketType, BaseMapDataVec> TypeToMapData;
	typedef std::map<MapID, TypeToMapData> MapIdToMapDataMap;
	MapIdToMapDataMap   mapid_query_map_;
};

///
/// template func
///
template<class T> 
void MapDataManager::QueryMapDataByType(MapID mapid, std::vector<const T*>& ptr_vec)
{
	MapIdToMapDataMap::const_iterator it_mapid = mapid_query_map_.find(mapid);
	if (it_mapid == mapid_query_map_.end())
	{
		//ASSERT(false && "没找到对应地图的数据it_mapid");
		return;
	}

	TypeToMapData::const_iterator it_type = it_mapid->second.find(T::TypeNumber());
	if (it_type == it_mapid->second.end())
	{
		//ASSERT(false && "没有找到对应type的地图数据it_type");
		return;
	}

	const BaseMapDataVec& baseptr_vec = it_type->second;
	for (size_t i = 0; i < baseptr_vec.size(); ++i)
	{
		ptr_vec.push_back(dynamic_cast<const T*>(baseptr_vec[i]));
	}
}

template<class T> 
const T* MapDataManager::QueryMapDataTempl(ElemID id)
{
	const BaseMapData* pbase_ptr = QueryBaseMapDataTempl(id);
	if (pbase_ptr == NULL)
		return NULL;
	return dynamic_cast<const T*>(pbase_ptr);
}

#define s_pMapData mapDataSvr::MapDataManager::GetInstance()

} // namespace gamed

#endif // GAMED_GS_TEMPLATE_MAPDATA_MAPDATA_MANAGER_H_
