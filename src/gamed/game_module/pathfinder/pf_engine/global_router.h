#ifndef __PF_GLOBAL_ROUTER_H__
#define __PF_GLOBAL_ROUTER_H__

#include <map>
#include <vector>
#include <stdint.h>
#include "point.h"
#include "dgraph.h"
#include "shared/base/mutex.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace net
}; // namespae shared

namespace pathfinder
{

class DGraph;

struct VexNode;
struct ArcNode;

/**
 * @class TransTable--全局传送表
 * @brief 每张地图有一定数量的出口和入口,不同地图的出入口相互连接。
 *        每个出口可以到达的目的地唯一，地图的入口可以共享，即不同地图可以传送到目标地图的同一入口。
 */
class TransTable
{
protected:
private:
	typedef int16_t MapID;     // 地图ID
	typedef int16_t MapInNum;  // 入口编号
	typedef int16_t MapOutNum; // 出口编号
	typedef std::vector<MapInNum>  MapInVec;
	typedef std::vector<MapOutNum> MapOutVec;

private:
	struct MapEntry
	{
		MapInVec  in_list;    // 入口列表
		MapOutVec out_list;   // 出口列表
	};

	typedef std::map<MapID, MapEntry> TransMap;
	TransMap _map;

	friend class GlobalTransDataGenerator;

public:
	TransTable();
	~TransTable();

	void Load(shared::net::Buffer& buffer);
	void Save(shared::net::Buffer& buffer);

	bool QueryWayIn(MapID mapid, MapInVec& list);
	bool QueryWayOut(MapID mapid, MapOutVec& list);
};


/**
 * @class GlobalRouter
 * @brief 全局路径路由器
 * @brief 确定跨地图时途径哪些地图和传送点
 * @brief 提供的接口为线程安全的
 */
class GlobalRouter
{
protected:
	typedef int32_t MapId;
	typedef DGraph TransGraph;

private:
	TransTable* _trans_table; // 全局传送表
	TransGraph* _trans_graph; // 全局传送图
	shared::MutexLock _self_lock;

public:
	GlobalRouter();
	~GlobalRouter();

	bool Init(shared::net::Buffer& buffer);
	void Release();
	bool SearchRoute(MapId src_mapid, const PointI& source, MapId dst_mapid, const PointI& dest, Route& route);/*thread safe*/

protected:
private:
	VexNode* InsertSrcPoint(MapId mapid, const PointI& pt);
	VexNode* InsertDestPoint(MapId mapid, const PointI& pt);
	bool     RemoveFromGraph(VexNode* node);
	bool     ConnectToBoarder(MapId mapid, VexNode* node, const std::vector<short>& list, bool is_start=true);
	bool     GenerateRoute(Route& route, const std::vector<const ArcNode*>& path) const;
};

};

#endif // __PF_GLOBAL_ROUTER_H__
