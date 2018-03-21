#include "game_stl.h"
#include "global_router.h"
#include "graph_astar.h"
#include "pf_engine.h"
#include "pf_centre.h"
#include "move_map.h"
#include "dgraph.h"
#include "utility.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/**********************************TransTable***********************************/
/**********************************TransTable***********************************/
/**********************************TransTable***********************************/
/**********************************TransTable***********************************/

TransTable::TransTable()
{
}

TransTable::~TransTable()
{
	TransMap::iterator it = _map.begin();
	for (; it != _map.end(); ++ it)
	{
		it->second.in_list.clear();
		it->second.out_list.clear();
	}

	_map.clear();
}

void TransTable::Load(shared::net::Buffer& buffer)
{
	int32_t map_count = buffer.ReadInt32();
	for (int i = 0; i < map_count; ++ i)
	{
		MapID mapid = buffer.ReadInt16();
		MapEntry& entry = _map[mapid];

		//加载入口信息
		int32_t in_count = buffer.ReadInt32();//入口个数
		entry.in_list.resize(in_count);
		for (int j = 0; j < in_count; ++ j)
		{
			entry.in_list[j] = buffer.ReadInt16();
		}

		//加载出口信息
		int32_t out_count = buffer.ReadInt32();//出口个数
		entry.out_list.resize(out_count);
		for (int k = 0; k < out_count; ++ k)
		{
			entry.out_list[k] = buffer.ReadInt16();
		}
	}
}

void TransTable::Save(shared::net::Buffer& buffer)
{
	int32_t map_count = _map.size();
	buffer.AppendInt32(map_count);

	TransMap::const_iterator it = _map.begin();
	for (; it != _map.end(); ++ it)
	{
		MapID mapid  = it->first;
		buffer.AppendInt16(mapid);

		const MapEntry& entry = it->second;

		//保存入口信息
		int32_t in_count = entry.in_list.size();
		buffer.AppendInt32(in_count);
		for (int i = 0; i < in_count; ++ i)
		{
			buffer.AppendInt16(entry.in_list[i]);
		}

		//保存出口信息
		int32_t out_count = entry.out_list.size();
		buffer.AppendInt32(out_count);
		for (int i = 0; i < out_count; ++ i)
		{
			buffer.AppendInt16(entry.out_list[i]);
		}
	}
}

bool TransTable::QueryWayIn(MapID mapid, MapInVec& list)
{
	TransMap::const_iterator it = _map.find(mapid);
	if (it == _map.end()) return false;
	list = it->second.in_list;
	return true;
}

bool TransTable::QueryWayOut(MapID mapid, MapOutVec& list)
{
	TransMap::const_iterator it = _map.find(mapid);
	if (it == _map.end()) return false;
	list = it->second.out_list;
	return true;
}


/**********************************GlobalRouter***********************************/
/**********************************GlobalRouter***********************************/
/**********************************GlobalRouter***********************************/
/**********************************GlobalRouter***********************************/

GlobalRouter::GlobalRouter():
	_trans_table(NULL),
	_trans_graph(NULL)
{
}

GlobalRouter::~GlobalRouter()
{
	assert(!_trans_table);
	assert(!_trans_graph);
}

bool GlobalRouter::Init(shared::net::Buffer& buffer)
{
	shared::MutexLockGuard lock(_self_lock);

	//加载传送图数据
	_trans_table = new TransTable();
	if (_trans_table)
	{
		_trans_table->Load(buffer);
	}
	else
	{
		return false;
	}

	_trans_graph = new DGraph();
	if (_trans_graph)
	{
		_trans_graph->Load(buffer);
	}
	else
	{
		return false;
	}

	assert(buffer.ReadableBytes() == 0);

	return true;
}

void GlobalRouter::Release()
{
	shared::MutexLockGuard lock(_self_lock);

	if (_trans_table)
	{
		delete _trans_table;
		_trans_table = NULL;
	}
	if (_trans_graph)
	{
		delete _trans_graph;
		_trans_graph = NULL;
	}
}

bool GlobalRouter::SearchRoute(MapId src_mapid, const PointI& source, MapId dst_mapid, const PointI& dest, Route& route)
{
	/**
	 * 跨地图寻路步骤:
	 * 1) 将起点和终点插入全局传送图,形成新图
	 * 2) 在新图上进行A*寻路,得到途径的地图和传送点
	 */

	shared::MutexLockGuard lock(_self_lock);

	//1) 插入起点和终点
	VexNode* __src_node = InsertSrcPoint(src_mapid, source);
	if (!__src_node)
	{
		return false;
	}
	VexNode* __dst_node = InsertDestPoint(dst_mapid, dest);
	if (!__dst_node)
	{
		RemoveFromGraph(__src_node);
		return false;
	}

	//2) 在传送图上寻路
	GAStar<DGraph, VexNode, ArcNode> gAstar;
	std::vector<const ArcNode*> __abs_path;
	if (!gAstar.Search(_trans_graph, __src_node, __dst_node, __abs_path))
	{
		RemoveFromGraph(__dst_node);
		RemoveFromGraph(__src_node);
		return false;
	}

	route.clear();
	if (!GenerateRoute(route, __abs_path))
	{
		RemoveFromGraph(__dst_node);
		RemoveFromGraph(__src_node);
		return false;
	}

	//3) 删除起点和终点
	RemoveFromGraph(__dst_node);
	RemoveFromGraph(__src_node);

	return true;
}

VexNode* GlobalRouter::InsertSrcPoint(MapId mapid, const PointI& pt)
{
	//获得本地图的所有出口
	std::vector<short> out_list;
	if (!_trans_table->QueryWayOut(mapid, out_list))
	{
		return NULL;
	}

	//创建起始节点
	VexNode* node = new VexNode(mapid, pt.x, pt.z);
	if (!node)
	{
		return NULL;
	}

	if (_trans_graph->AddVetex(node) < 0)
	{
		delete node;
		return NULL;
	}

	//将起点与本地图的出口连通
	if (!ConnectToBoarder(mapid, node, out_list))
	{
		RemoveFromGraph(node);
		delete node;
		return NULL;
	}
	
	return node;
}

VexNode* GlobalRouter::InsertDestPoint(MapId mapid, const PointI& pt)
{
	//获得本地图的所有入口
	std::vector<short> in_list;
	if (!_trans_table->QueryWayIn(mapid, in_list))
	{
		return NULL;
	}

	//创建起始节点
	VexNode* node = new VexNode(mapid, pt.x, pt.z);
	if (!node)
	{
		return NULL;
	}

	if (_trans_graph->AddVetex(node) < 0)
	{
		delete node;
		return NULL;
	}

	//将终点与本地图的入口连通
	if (!ConnectToBoarder(mapid, node, in_list, false))
	{
		RemoveFromGraph(node);
		delete node;
		return NULL;
	}

	return node;
}

bool GlobalRouter::ConnectToBoarder(MapId mapid, VexNode* node, const std::vector<short>& list, bool is_start)
{
	if (!node || list.empty())
	{
		return false;
	}

	MoveMap* movemap = PfCentre::GetInstance()->AllocWalkMap(mapid);
	if (!movemap)
	{
		assert(false);
		return false;
	}

	PfEngine engine(movemap);
	const VexNode* phead = NULL;
	const VexNode* ptail = NULL;

	for (size_t i = 0; i < list.size(); ++ i)
	{
		short vex_num = list[i];
		const VexNode* next = _trans_graph->GetNode(vex_num);
		if (!next)
		{
			return false;
		}

		if (is_start)
		{
			phead = next;
			ptail = node;
		}
		else
		{
			phead = node;
			ptail = next;
		}

		PointI __start = ptail->GetCoord();
		PointI __goal  = phead->GetCoord();

		float  __dist = 0.0f;
		engine.Setup(__start, __goal);
		if (engine.Distance(__dist) != PF_ENGINE_STATE_FOUND)
		{
			continue;
		}

		ArcNode* edge = new ArcNode(__dist, ptail->GetIndex(), phead->GetIndex());
		if (!edge)
		{
			return false;
		}

		if (_trans_graph->AddArc(edge) < 0)
		{
			delete edge;
			return false;
		}

		engine.Cleanup();
	}

	PfCentre::GetInstance()->FreeWalkMap(mapid, movemap);
	return true;
}

bool GlobalRouter::RemoveFromGraph(VexNode* node)
{
	bool rst = _trans_graph->RmvVetex(node);
	assert(rst);
	return rst;
}

/**
 * Route路由包含起点和终点,类似下面的形式
 * 起点-->>Map1出口-->>Map2入口-->>Map2出口 ......  MapN-1的入口-->>MapN-1的出口-->>MapN的入口-->>终点
 * Map1--起始地图
 * MapN--目标地图
 */
bool GlobalRouter::GenerateRoute(Route& route, const std::vector<const ArcNode*>& path) const
{
	assert(path.size() > 1);

	route.clear();
	route.push_back(_trans_graph->GetNode(path.front()->tail_vex));
	for (size_t i = 0; i < path.size(); ++ i)
	{
		const ArcNode* edge = path[i];
		route.push_back(_trans_graph->GetNode(edge->head_vex));
	}
	return true;
}

};
