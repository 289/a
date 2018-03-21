#ifndef __PF_ENGINE_H__
#define __PF_ENGINE_H__

#include "point.h"
#include "dgraph.h"
#include "path_track.h"

namespace pathfinder
{

struct VexNode;
struct ArcNode;
class DGraph;
class BitMap;
class MoveMap;
class PathTrack;
class Cluster;

class PfEngine
{
private:
	MoveMap*   _movemap;         //通路图
	PointI     _start;           //寻径起点
	PointI     _goal;            //寻径终点
	char       _state;           //寻路系统状态
	bool       _active;          //是否在使用
	int32_t    _nodes_expanded;  //已遍历节点数

public:
	PfEngine(MoveMap* movemap);
	virtual ~PfEngine();

	void  Setup(const PointI& start, const PointI& goal);
	int   Search(Path& path);  //搜索具体路径
	int   Search(Route& route);//搜索途径路由
	int   Distance(float& dis);//获取最短距离
	void  Cleanup();
	void  Release();

	void  SetActive();
	bool  IsActive() const;

	int   GetState() const;
	int   GetNodesExpanded() const;

private:
	/**
	 * @func  RgnSearch
	 * @brief 在指定区域搜索
	 * @param map   通路图
	 * @param cluster_index 在哪个簇上搜索
	 * @param start 起点
	 * @param goal  终点
	 * @param path  寻径结果
	 * @param node_expanded 遍历节点数
	 * @ret   搜索结果
	 */
	int RgnSearch(const BitMap* map, const Cluster* cluster, const PointI& start, const PointI& goal, Path& path, int32_t& node_expanded);

	/**
	 * @func  AbsSearch
	 * @brief 在抽象图寻路
	 * @param graph 图结构
	 * @param start 起点
	 * @param goal  终点
	 * @param route 寻径结果
	 * @param node_expanded 遍历节点数
	 * @ret   搜索结果
	 */
	int AbsSearch(const DGraph* graph, const VexNode* start, const VexNode* goal, Route& route, int32_t& node_expanded);

    void CheckLineTo(Path& path, size_t begin, size_t end);
    void SmoothPath(Path& path);

	VexNode* InsertStartPoint(const Cluster* cluster, const PointI& pt);
	VexNode* InsertGoalPoint(const Cluster* cluster, const PointI& pt);
	bool RemoveFromDGraph(VexNode* node);
	bool ConnectToBorder(const VexNode* node, std::vector<int16_t>& nodes, const Cluster* cluster, bool is_start=true);
	bool GenerateRoute(Route& route, const std::vector<const ArcNode*>& path) const;
	bool SetFakeGoal(PointI& fake_goal);
#ifdef _DEBUG
    void DbgDumpPath(const char* szFilename, const Path& path);
#endif // #ifdef _DEBUG
};


///
/// inline func
///
inline void PfEngine::SetActive()
{
	_active = true;
}

inline bool PfEngine::IsActive() const
{
	return _active;
}

inline int PfEngine::GetState() const
{
	return _state;
}

inline int32_t PfEngine::GetNodesExpanded() const
{
	return _nodes_expanded;
}

};

#endif // __PF_ENGINE_H__
