#ifndef __PF_MASTAR_H__
#define __PF_MASTAR_H__

#include <stdio.h>
#include "point.h"
#include "types.h"
#include "base_astar.h"
#include "bit_map.h"
#include "pf_common.h"

namespace pathfinder
{

struct MapNode
{
	float cost;
	PointI point;
	MapNode* parent;

	MapNode()
		: cost(0.0f)
		, point(0,0)
		, parent(NULL)
	{
	}

	~MapNode()
	{
		cost = 0.0f;
		point = PointI(-1,-1);
		parent = NULL;
	}

	bool operator == (const MapNode& rhs) const
	{
		return point == rhs.point;
	}

	inline bool  Equal(const PointI& pt) const  { return pt == point; }
	inline short GetCorrdX() const              { return point.x; }
	inline short GetCorrdZ() const              { return point.z; }
	inline short GetOwner() const               { return 0; }

	inline float GetCost() const                { return cost; }
	inline const PointI& GetCoord() const       { return point; }
	inline MapNode* GetParent() const           { return parent; }

	inline void SetCost(float weight)           { if (weight <= 0) return; cost = weight; }
	inline void SetCorrd(const PointI& pt)      { point = pt; }
	inline void SetParent(MapNode* node)        { if (!node) return; parent = node; }
	inline void dump() const
	{
		printf("cost(%f),point(%d,%d),parent(%p)\n", cost, point.x, point.z, parent);
	}
};

/**
 * MapAStar---MAStar
 * 在原始地图上执行的A*
 */

class BitMap;
class Cluster;
class MAStar
{
protected:
private:
	int32_t _nodes_touched;
	int32_t _nodes_expanded;
	int32_t _max_expanded;
	OpenHeap<MapNode>  _openlist;
	CloseHash<MapNode> _closelist;

public:
	MAStar();
	~MAStar();

  // 这个函数地图编辑器传入了自定义的Range对象，所以不能假定range的类型。
	template<class RANGE>
	bool Search(const BitMap* bitmap, const PointI& start, const PointI& goal, Path& path, const RANGE* range=NULL)
  {
    //path.clear();
    /*
    if (!bitmap || !bitmap->IsPassable(start) || !bitmap->IsPassable(goal))
    {
    assert(false);
    return false;
    }
    */

    if (!bitmap)
    {
      assert(false);
      return false;
    }

    if (!bitmap->IsPassable(start))
    {
      PF2DVECTOR __start = bitmap->TransMap2World(start);
      PF2DVECTOR __goal  = bitmap->TransMap2World(goal);
      fprintf(stdout, "mapid=%d, start isn't passable, start=(%f,%f), goal=(%f,%f)\n", bitmap->GetMapID(), __start.x, __start.z, __goal.x, __goal.z);
      assert(false);
      return false;
    }

    if (!bitmap->IsPassable(goal))
    {
      PF2DVECTOR __start = bitmap->TransMap2World(start);
      PF2DVECTOR __goal  = bitmap->TransMap2World(goal);
      fprintf(stdout, "mapid=%d, goal isn't passable, start=(%f,%f), goal=(%f,%f)\n", bitmap->GetMapID(), __start.x, __start.z, __goal.x, __goal.z);
      assert(false);
      return false;
    }

    if (start == goal)
    {
      path.push_back(goal);
      return true;
    }

    //将起始位置加入OPEN列表
    MapNode* pCurNode = new MapNode();
    if (!pCurNode)
    {
      return false;
    }

    pCurNode->SetCost(HeuristicDist(start, goal));
    pCurNode->SetCorrd(start);
    pCurNode->SetParent(NULL);
    _openlist.Push(pCurNode);

    //循环处理OPEN列表中的节点
    while (!_openlist.Empty())
    {
      ++ _nodes_expanded;
      if (_nodes_expanded >= _max_expanded)
      {
        //fprintf(stdout, "mastar::nodes_touched\t= %d\n", _nodes_touched);
        //fprintf(stdout, "mastar::nodes_expanded\t= %d\n", _nodes_expanded);
        //fprintf(stdout, "mastar::search failed, exceed max-nodes, start=(%d,%d), goal=(%d,%d)\n\n", start.x, start.z, goal.x, goal.z);

        _openlist.Clear();
        _closelist.Clear();
        return false;
      }

      pCurNode = _openlist.Top();
      if (pCurNode->Equal(goal))
      {
        break;
      }

      _openlist.Pop();

      for (size_t i = 0; i < PF_NEIGHBOR_COUNT; ++ i)
      {
        PointI __pt = pCurNode->GetCoord();
        __pt.Offset(PF2D_NeighborDist[i*2], PF2D_NeighborDist[i*2+1]);
        if (!bitmap->IsPassable(__pt))
          continue; //禁止通行

        if (range && !range->TestRange(__pt))
          continue; //超过范围

        ++ _nodes_touched;
#ifdef _DEBUG
        const_cast<BitMap*>(bitmap)->SetChar(__pt, '#');
#endif // _DEBUG

        MapNode  neighbor;
        neighbor.SetCorrd(__pt);
        neighbor.SetCost(pCurNode->GetCost() + PF2D_NeighborCost[i]/* + HeuristicDist(neighbor.GetCoord(), goal)*/);
        neighbor.SetParent(pCurNode);

        MapNode* pNode = NULL;
        if ((pNode = _openlist.Find(neighbor)) != NULL)
        {
          if (neighbor.GetCost() < pNode->GetCost())
          {
            pNode->SetCost(neighbor.GetCost());
            pNode->SetParent(pCurNode);
            _openlist.Update(pNode);
          }
        }
        else if ((pNode = _closelist.Find(neighbor)) != NULL)
        {
          if (neighbor.GetCost() < pNode->GetCost())
          {
            _closelist.Remove(pNode);
            pNode->SetCost(neighbor.GetCost());
            pNode->SetParent(pCurNode);
            _openlist.Push(pNode);
          }
        }
        else
        {
          MapNode* pNewNode = new MapNode();
          if (!pNewNode)
          {
            _openlist.Clear();
            _closelist.Clear();
            return false;
          }

          *pNewNode = neighbor;
          _openlist.Push(pNewNode);
        }
      }

      _closelist.Push(pCurNode);
    };

    if (_openlist.Empty())
    {
      //fprintf(stdout, "mastar::nodes_touched\t= %d\n", _nodes_touched);
      //fprintf(stdout, "mastar::nodes_expanded\t= %d\n", _nodes_expanded);
      //fprintf(stdout, "mastar::search failed, no path, start=(%d,%d), goal=(%d,%d)\n\n", start.x, start.z, goal.x, goal.z);
      _openlist.Clear();
      _closelist.Clear();
      return false;
    }

    if (!BuildPath(pCurNode, path))
    {
      _openlist.Clear();
      _closelist.Clear();
      return false;
    }

    //fprintf(stdout, "mastar ok, start=(%d,%d), goal=(%d,%d), touched:%d, expaned:%d\n", start.x, start.z, goal.x, goal.z, _nodes_touched, _nodes_expanded);

    //fprintf(stdout, "mastar::nodes_touched\t= %d\n", _nodes_touched);
    //fprintf(stdout, "mastar::nodes_expanded\t= %d\n", _nodes_expanded);
    //fprintf(stdout, "mastar::search succ, start=(%d,%d), goal=(%d,%d)\n", start.x, start.z, goal.x, goal.z);
    //for (size_t i = 0; i < path.size(); ++ i)
    //{
    //	fprintf(stdout, "(%d,%d),", path[i].x, path[i].z);
    //}
    //fprintf(stdout, "\n");

    //_openlist.Clear();
    //_closelist.Clear();
    return true;
  }


	int32_t GetNodesExpanded() const;

private:
	bool BuildPath(const MapNode* dest, Path& path) const;
};

};

#endif // __PF_MASTAR_H__
