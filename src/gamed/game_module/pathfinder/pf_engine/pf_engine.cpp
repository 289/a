#include <limits.h>
#include "game_stl.h"
#include "pf_engine.h"
#include "dgraph.h"
#include "move_map.h"
#include "pf_common.h"
#include "map_astar.h"
#include "graph_astar.h"
#include "cluster.h"
#include "utility.h"
#include "expander.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }
#endif // #ifndef SAFE_DELETE_ARRAY

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] p; p = NULL; }
#endif // #ifndef SAFE_DELETE_ARRAY

namespace pathfinder
{

/**************************************PfEngine************************************/
/**************************************PfEngine************************************/
/**************************************PfEngine************************************/
/**************************************PfEngine************************************/

PfEngine::PfEngine(MoveMap* movemap) :
	_movemap(movemap),
	_state(PF_ENGINE_STATE_IDLE),
	_active(false),
	_nodes_expanded(0)
{
}

PfEngine::~PfEngine()
{
	_movemap = NULL;
}

void PfEngine::Setup(const PointI& start, const PointI& goal)
{
	_start   = start;
	_goal    = goal;
	_state   = PF_ENGINE_STATE_SETUP;
}

void PfEngine::Cleanup()
{
	_start          = PointI(-1,-1);
	_goal           = PointI(-1,-1);
	_state          = PF_ENGINE_STATE_IDLE;
	_active         = false;
	_nodes_expanded = 0;
}

int PfEngine::Search(Path& path)
{
	PointI __start = _start;
	PointI __goal  = _goal;

	if (!_movemap->IsPassable(__start))
	{
		return PF_ENGINE_STATE_ERROR;
	}

	if (!_movemap->IsPassable(__goal))
	{
		//assert(false);
		//return PF_ENGINE_STATE_ERROR;

		//can't reach goal, set fake-goal
		PointI __fake_goal;
		if (!SetFakeGoal(__fake_goal))
		{
			//set fake-goal failed.
			return PF_ENGINE_STATE_ERROR;
		}

		__goal = __fake_goal;
	}

	if (_movemap->LinePath2(__start, __goal, path))
	{
		_state = PF_ENGINE_STATE_FOUND;
		return _state;
	}

    path.clear();
    //if( ! path.empty()) {
    //    __start = path.back();
    //}

	const Cluster* pClusterSrc  = _movemap->LocateCluster(__start);
	const Cluster* pClusterDest = _movemap->LocateCluster(__goal);
	if (!pClusterSrc || !pClusterDest)
	{
		assert(false);
		_state = PF_ENGINE_STATE_ERROR;
		return _state;
	}

	if (pClusterSrc != pClusterDest)
	{
		assert(false);
		_state = PF_ENGINE_STATE_ERROR;
		return _state;
	}

	int nodes_expanded = 0;
	int state = RgnSearch(_movemap->GetBitMap(), pClusterSrc, __start, __goal, path, nodes_expanded);
    SmoothPath(path);
	_state = state;
	_nodes_expanded += nodes_expanded;
	return _state;
}

int PfEngine::Search(Route& route)
{
	/**
	 * 如果起点和终点位于同一簇，将直接返回起点和终点
	 * 如果起点和终点位于不同簇，则在抽象图上搜索路由
	 */

	PointI __start = _start;
	PointI __goal  = _goal;

	if (!_movemap->IsPassable(__start))
	{
		return PF_ENGINE_STATE_ERROR;
	}

	if (!_movemap->IsPassable(__goal))
	{
		//can't reach goal, set fake-goal
		PointI __fake_goal;
		if (!SetFakeGoal(__fake_goal))
		{
			//set fake-goal failed.
			return PF_ENGINE_STATE_ERROR;
		}

		__goal = __fake_goal;
	}

	const Cluster* pClusterSrc  = _movemap->LocateCluster(__start);
	const Cluster* pClusterDest = _movemap->LocateCluster(__goal);
	if (!pClusterSrc || !pClusterDest)
	{
		return PF_ENGINE_STATE_ERROR;
	}

	//位于同一簇
	if (pClusterSrc == pClusterDest)
	{
		VexNode* src_node  = new VexNode(pClusterSrc->GetIndex(), __start.x, __start.z);
		if (!src_node)
		{
			return PF_ENGINE_STATE_ERROR;
		}

		VexNode* dest_node = new VexNode(pClusterDest->GetIndex(), __goal.x, __goal.z);
		if (!dest_node)
		{
			delete src_node;
			return PF_ENGINE_STATE_ERROR;
		}

		route.push_back(src_node);
		route.push_back(dest_node);
		return PF_ENGINE_STATE_FOUND;
	}

	//位于不同簇
	//1) 将起点和终点插入图
	VexNode* __src_node  = InsertStartPoint(pClusterSrc, __start);
	if (!__src_node)
	{
		return PF_ENGINE_STATE_ERROR;
	}
	VexNode* __dest_node = InsertGoalPoint(pClusterDest, __goal);
	if (!__dest_node)
	{
		RemoveFromDGraph(__src_node);
        SAFE_DELETE(__src_node);
		return PF_ENGINE_STATE_ERROR;
	}

	//2) 在图上进行A*搜索
	int nodes_expanded = 0;
	int state = AbsSearch(_movemap->GetGraph(), __src_node, __dest_node, route, nodes_expanded);
	if (state != PF_ENGINE_STATE_FOUND)
	{
		RemoveFromDGraph(__dest_node);
		RemoveFromDGraph(__src_node);
        SAFE_DELETE(__dest_node);
        SAFE_DELETE(__src_node);
		return state;
	}

	_nodes_expanded += nodes_expanded;

	//3) 删除新插入的节点
	RemoveFromDGraph(__dest_node);
	RemoveFromDGraph(__src_node);
  //SAFE_DELETE(__dest_node);
  //SAFE_DELETE(__src_node);

	return PF_ENGINE_STATE_FOUND;
}

int PfEngine::Distance(float& dist)
{
	PointI __start = _start;
	PointI __goal  = _goal;

	if (!_movemap->IsPassable(__start))
	{
		return PF_ENGINE_STATE_ERROR;
	}

	if (!_movemap->IsPassable(__goal))
	{
		//can't reach goal, set fake-goal
		PointI __fake_goal;
		if (!SetFakeGoal(__fake_goal))
		{
			//set fake-goal failed.
			return PF_ENGINE_STATE_ERROR;
		}
		__goal = __fake_goal;
	}

	const Cluster* pClusterSrc  = _movemap->LocateCluster(__start);
	const Cluster* pClusterDest = _movemap->LocateCluster(__goal);
	if (!pClusterSrc || !pClusterDest)
	{
		return PF_ENGINE_STATE_ERROR;
	}

	//位于同一簇
	if (pClusterSrc == pClusterDest)
	{
		Path __path;
		int nodes_expanded = 0;
    if(_movemap->LinePath2(__start, __goal, __path))
    {
      return PF_ENGINE_STATE_FOUND;
    }

		int state = RgnSearch(_movemap->GetBitMap(), pClusterSrc, __start, __goal, __path, nodes_expanded);
		if (state != PF_ENGINE_STATE_FOUND)
			return state;

		dist = pathfinder::Distance(__path);
		return PF_ENGINE_STATE_FOUND;
	}

	//位于不同簇
	//1) 将起点和终点插入图
	VexNode* __src_node  = InsertStartPoint(pClusterSrc, __start);
	if (!__src_node)
	{
		return PF_ENGINE_STATE_ERROR;
	}
	VexNode* __dest_node = InsertGoalPoint(pClusterDest, __goal);
	if (!__dest_node)
	{
		RemoveFromDGraph(__src_node);
        SAFE_DELETE(__src_node);
		return PF_ENGINE_STATE_ERROR;
	}

	//2) 在图上进行A*搜索
	std::vector<const ArcNode*> __abs_path;
	GAStar<DGraph, VexNode, ArcNode> gAStar;
	if (!gAStar.Search(_movemap->GetGraph(), __src_node, __dest_node, __abs_path))
	{
		RemoveFromDGraph(__dest_node);
		RemoveFromDGraph(__src_node);
        SAFE_DELETE(__dest_node);
        SAFE_DELETE(__src_node);
		return PF_ENGINE_STATE_NOPATH;
	}

	// 计算抽象路径的距离
	float  __dist = 0.0f;
	for (size_t i = 0; i < __abs_path.size(); i += 2)
	{
		__dist += __abs_path[i]->GetWeight();
	}

	//3) 删除新插入的节点
	RemoveFromDGraph(__dest_node);
	RemoveFromDGraph(__src_node);

    SAFE_DELETE(__dest_node);
    SAFE_DELETE(__src_node);

	return PF_ENGINE_STATE_FOUND;
}

int PfEngine::RgnSearch(const BitMap* map, const Cluster* cluster, const PointI& start, const PointI& goal, Path& path, int32_t& nodes_expanded)
{
	MAStar mAStar;
	if (!mAStar.Search(map, start, goal, path, cluster))
	{
		return PF_ENGINE_STATE_NOPATH;
	}

	nodes_expanded = mAStar.GetNodesExpanded();
	return PF_ENGINE_STATE_FOUND;
}

int PfEngine::AbsSearch(const DGraph* graph, const VexNode* start, const VexNode* goal, Route& route, int32_t& nodes_expanded)
{
	GAStar<DGraph, VexNode, ArcNode> gAStar;
	std::vector<const ArcNode*> __abs_path;
	if (!gAStar.Search(graph, start, goal, __abs_path))
	{
		return PF_ENGINE_STATE_NOPATH;
	}

	if (!GenerateRoute(route, __abs_path))
	{
		return PF_ENGINE_STATE_ERROR;
	}

	nodes_expanded = gAStar.GetNodesExpanded();
	return PF_ENGINE_STATE_FOUND;
}

VexNode* PfEngine::InsertStartPoint(const Cluster* cluster, const PointI& pt)
{
	assert(cluster && cluster->TestRange(pt));

	//获得本簇所有出口
	std::vector<int16_t> out_list = cluster->GetOutList();
	if (out_list.empty())
	{
		return NULL;
	}

	//创建图节点
	VexNode* node = new VexNode(cluster->GetIndex(), pt.x, pt.z);
	if (!node)
	{
		return NULL;
	}

	//插入新节点
	if (_movemap->GetGraph()->AddVetex(node) < 0)
	{
		delete node;
		return NULL;
	}

	//将起点与本簇的出口连通
	if (!ConnectToBorder(node, out_list, cluster))
	{
		RemoveFromDGraph(node);
		delete node;
		return NULL;
	}

	return node;
}

VexNode* PfEngine::InsertGoalPoint(const Cluster* cluster, const PointI& pt)
{
	assert(cluster && cluster->TestRange(pt));

	//获得本簇所有入口
	std::vector<int16_t> in_list = cluster->GetInList();
	if (in_list.empty())
	{
		return NULL;
	}

	//创建图节点
	VexNode* node = new VexNode(cluster->GetIndex(), pt.x, pt.z);
	if (!node)
	{
		return NULL;
	}

	//插入新节点
	if (_movemap->GetGraph()->AddVetex(node) < 0)
	{
		delete node;
		return NULL;
	}

	//将终点与本簇的入口连通
	if (!ConnectToBorder(node, in_list, cluster, false))
	{
		_movemap->GetGraph()->RmvVetex(node);
		delete node;
		return NULL;
	}

	return node;
}

bool PfEngine::RemoveFromDGraph(VexNode* node)
{
	bool rst = _movemap->GetGraph()->RmvVetex(node);
	assert(rst);
	return rst;
}

bool PfEngine::ConnectToBorder(const VexNode* node, std::vector<int16_t>& list, const Cluster* cluster, bool is_start)
{
	if (!node || list.empty())
	{
		return false;
	}

	const VexNode* phead = NULL;
	const VexNode* ptail = NULL;

	DGraph* graph = _movemap->GetGraph();
	assert(graph);

	for (size_t i = 0; i < list.size(); ++ i)
	{
		short vex_num = list[i];
		const VexNode* next = graph->GetNode(vex_num);
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

		Path __path;
		PointI __source = ptail->GetCoord();
		PointI __dest   = phead->GetCoord();

		int nodes_expanded = 0;
		int state = RgnSearch(_movemap->GetBitMap(), cluster, __source, __dest, __path, nodes_expanded);
		if (state != PF_ENGINE_STATE_FOUND)
		{
			continue;
		}

		ArcNode* edge = new ArcNode(pathfinder::Distance(__path), ptail->GetIndex(), phead->GetIndex());
		if (!edge)
		{
			return false;
		}

		if (graph->AddArc(edge) < 0)
		{
			delete edge;
			return false;
		}
	}

	return true;
}

/**
 * @brief 由抽象路径生成沿途经过的节点,包含起点和终点
 */
bool PfEngine::GenerateRoute(Route& route, const std::vector<const ArcNode*>& path) const
{
	assert(path.size() > 1);

	DGraph* graph = _movemap->GetGraph();
	assert(graph);

	route.clear();
	route.push_back(graph->GetNode(path.front()->tail_vex));
	for (size_t i = 0; i < path.size(); ++ i)
	{
		const ArcNode* edge = path[i];
		route.push_back(graph->GetNode(edge->head_vex));
	}
	return true;
}

/**
 * @brief 在本地图寻路时，如果终点不可达，则使用虚假目标来代替
 * @brief 虚假目标定义：从终点到起点的矢量记为SG，将矢量SG顺时针旋转45度得到矢量SG1，逆时针旋转45度得到矢量SG2,
 *        分别沿矢量SG/SG1/SG2前进，直至遇到可以通行的点，三个矢量往前延伸的距离分别为Dist,Dist1,Dist2,
 *        取距离最短的为虚假目标.
*/
bool PfEngine::SetFakeGoal(PointI& fake_goal)
{

#if 1
#define MOVE_UNTIL_PASSABLE(num) \
	{ \
		PointI __cur_pt; \
		int32_t count = 0; \
		while ((float)(count) < __len) \
		{ \
			++ count; \
			__cur_pt.x = __goal.x + count * __dir##num.x; \
			__cur_pt.z = __goal.z + count * __dir##num.z; \
			if (_movemap->IsPassable(__cur_pt)) \
			{ \
				float __tmp_dist = (__cur_pt - __goal).Magnitude(); \
				if (__tmp_dist < __min_dist) \
				{ \
					__found = true; \
					__fake_goal = __cur_pt; \
					__min_dist = __tmp_dist; \
					break; \
				} \
			} \
		}; \
	}

	PointI __start = _start;
	PointI __goal  = _goal;

	PointI tmp = __start - __goal;
	PointF dir = PointF((float)(tmp.x), (float)(tmp.z));

	PointF __dir1 = dir;
	PointF __dir2 = ClockWiseRotateVector(dir, 45);
	PointF __dir3 = AntiClockWiseRotateVector(dir, 45);

	__dir1.Normalization();
	__dir2.Normalization();
	__dir3.Normalization();

	bool  __found = false;
	float __len = 1.0f + dir .Magnitude();
	float __min_dist = (float)(INT_MAX);
	PointI __fake_goal;

	MOVE_UNTIL_PASSABLE(1);
	MOVE_UNTIL_PASSABLE(2);
	MOVE_UNTIL_PASSABLE(3);
#undef MOVE_UNTIL_PASSABLE

	if (!__found)
	{
		return false;
	}

	assert(_movemap->IsPassable(__fake_goal));
	fake_goal = __fake_goal;
	return true;

#else

	PointI __fake_goal;
	PointI __goal = _goal;

	Expander expander;
	if (expander.Search(_movemap->GetBitMap(), __goal, __fake_goal))
	{
		fake_goal = __fake_goal;
		return true;
	}

	return false;
#endif
}

void PfEngine::SmoothPath( Path& path )
{
    const size_t nStep = 10;
//#ifdef _DEBUG
//    DbgDumpPath("before_smooth.txt", path);
//#endif // #ifdef _DEBUG
    for (size_t i = 0; i < path.size(); i += nStep)
    {
        if(i + nStep < path.size()) {
            CheckLineTo(path, i, i + nStep);
        }
    }
//#ifdef _DEBUG
//    DbgDumpPath("after_smooth.txt", path);
//#endif // #ifdef _DEBUG
}

void PfEngine::CheckLineTo( Path& path, size_t begin, size_t end )
{
    assert(begin < end);
    assert(end < path.size());
    Path sub;
    if(_movemap->LinePath2(path[begin], path[end], sub))
    {
        size_t count = end - begin + 1;
        if(count > sub.size()) {
            path.erase(path.begin() + begin, path.begin() + begin + count - sub.size());
        }
        else if(count < sub.size()) {
            path.insert(path.begin() + begin, sub.size() - count, sub.front());
        }
        
        Path::iterator itDest = path.begin() + begin;
        for(Path::iterator it = sub.begin(); it != sub.end(); ++it, ++itDest)
        {
            *itDest = *it;
        }
        //path.insert(path.begin() + begin, sub.begin(), sub.end());
    }
}

#ifdef _DEBUG
void PfEngine::DbgDumpPath( const char* szFilename, const Path& path )
{
    FILE* fp = fopen(szFilename, "wt");
    for(Path::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        fprintf(fp, "%d,%d\n", it->x, it->z);
    }
    fclose(fp);
}
#endif // #ifdef _DEBUG

} // namespace pathfinder
