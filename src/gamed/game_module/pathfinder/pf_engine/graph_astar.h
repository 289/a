#ifndef __PF_GASTAR_H__
#define __PF_GASTAR_H__

#include <stdio.h>
#include "base_astar.h"
#include "pf_common.h"
#include "clock.h"

namespace pathfinder
{

/**
 * @class GAStar--GraphAStar
 * @brief A*算法在抽象图上的实现
 * @brief 应用场景：
 *        1) 地图内寻路：当地图较大时，直接用A*算法寻路消耗过大.
 *           可以将大地图切割成多个块，每个块称为一个"簇"。簇与簇之间通过出入口连接，出入口的相互连接构成一张无向图。
 *        2) 地图间寻路：玩家跨地图寻路。
 *           每张地图有自己的出入口，不同地图的出入口中间相互联通构成一张有向图,记为DGraph.
 *           通过在DGraph上进行A*算法，得到从起点到终点需要经过哪些地图以及出入口。
 */
template<class GRAPH, class GNODE, class GEDGE>
class GAStar
{
private:
	/**
	 * @class ABSNode--Abstract Node
	 * @brief 抽象节点，在抽象图上进行A*时使用
	 */
	struct ABSNode
	{
		const GNODE* node;
		const GEDGE* edge;
		float cost;
		ABSNode* parent;
	
		ABSNode():
			node(NULL),
			edge(NULL),
			cost(0.0f),
			parent(NULL)
		{
		}
		virtual ~ABSNode()
		{
			node = NULL;
			edge = NULL;
			cost = 0.0f;
			parent = NULL;
		}
	
		bool operator == (const ABSNode& rhs) const
		{
			return *(rhs.node) == *node;
		}
	
		inline bool Equal(const GNODE* p) const
		{
			if (!p) return false;
			return *(this->node) == *p;
		}
	
		inline void dump() const
		{
			fprintf(stdout, "cost(%f), coord(%d,%d)\n", cost, node->GetCoord().x, node->GetCoord().z);
		}

		inline float GetCost() const        { return cost; }
		inline const GNODE* GetNode() const { return node; }
		inline const GEDGE* GetEdge() const { return edge; }
		inline ABSNode* GetParent() const   { return parent; }
		inline PointI GetCoord() const      { return node->GetCoord(); }
		inline short  GetOwner() const      { return node->GetOwner(); }

		inline void SetCost(float weight)   { if (weight <= 0) return; cost = weight; }
		inline void SetNode(const GNODE* p) { if (!p) return; node = p; }
		inline void SetEdge(const GEDGE* p) { if (!p) return; edge = p; }
		inline void SetParent(ABSNode* p)   { if (!p) return; parent = p; }
	};

private:
	typedef std::vector<const GEDGE*> ABSPath;

private:
	int32_t _nodes_touched;
	int32_t _nodes_expanded;
	int32_t _max_expanded;
	OpenHeap<ABSNode>  _openlist;
	CloseHash<ABSNode> _closelist;

public:
	GAStar():
		_nodes_touched(0),
		_nodes_expanded(0),
		_max_expanded(1000)
	{
	}
	~GAStar()
	{
		_openlist.Clear();
		_closelist.Clear();
	}

	bool Search(const GRAPH* graph, const GNODE* start, const GNODE* goal, ABSPath& abs_path)
	{
		if (!graph || !start || !goal)
		{
			assert(false);
			return false;
		}

		if (*start == *goal)
		{
			return true;
		}

		//test
		//Clock clock("GAStar");
		//clock.Start();
	
		//将起始位置加入OPEN列表
		ABSNode* pCurNode = new ABSNode();
		if (!pCurNode)
		{
			return false;
		}
	
		pCurNode->SetNode(start);
		pCurNode->SetCost(GetHeuristic(start, goal));
		pCurNode->SetEdge(NULL);
		pCurNode->SetParent(NULL);
		_openlist.Push(pCurNode);
	
		//循环处理OPEN列表中的节点
		while (!_openlist.Empty())
		{
			++ _nodes_expanded;
			if (_nodes_expanded >= _max_expanded)
			{
				//fprintf(stdout, "gastar::nodes_touched\t= %d\n", _nodes_touched);
				//fprintf(stdout, "gastar::nodes_expanded\t= %d\n", _nodes_expanded);
				//fprintf(stdout, "gastar::search failed, exceed max-nodes, start=(%d,%d), goal=(%d,%d)\n\n", start->GetCoord().x, start->GetCoord().z, goal->GetCoord().x, goal->GetCoord().z);
				return false;
			}
	
			pCurNode = _openlist.Top();
			if (pCurNode->Equal(goal))
			{
				break;
			}
	
			_openlist.Pop();

			//遍历所有相邻的节点
			std::vector<GEDGE*> edges = pCurNode->GetNode()->GetNeighbours();
			for (size_t i = 0; i < edges.size(); ++ i)
			{
				++ _nodes_touched;
	
				const GEDGE* edge = edges[i];
				int32_t peer = pCurNode->GetNode()->GetIndex() == edge->GetFrom() ? edge->GetTo() : edge->GetFrom();
				const GNODE* next_node = graph->GetNode(peer);
				assert(next_node);
	
				ABSNode neighbor;
				neighbor.SetNode(next_node);
				neighbor.SetEdge(edge);
				neighbor.SetCost(pCurNode->GetCost() + edge->GetWeight() + GetHeuristic(next_node, goal));
				neighbor.SetParent(pCurNode);
	
				ABSNode* pNode = NULL;
				if ((pNode = _openlist.Find(neighbor)) != NULL)
				{
					if (neighbor.GetCost() < pNode->GetCost())
					{
						pNode->SetEdge(edge);
						pNode->SetCost(neighbor.cost);
						pNode->SetParent(pCurNode);
						_openlist.Update(pNode);
					}
				}
				else if ((pNode = _closelist.Find(neighbor)) != NULL)
				{
					if (neighbor.GetCost() < pNode->GetCost())
					{
						_closelist.Remove(pNode);
						pNode->SetEdge(edge);
						pNode->SetCost(neighbor.cost);
						pNode->SetParent(pCurNode);
						_openlist.Push(pNode);
					}
				}
				else
				{
					ABSNode* pNewNode = new ABSNode();
					if (!pNewNode)
					{
						return false;
					}

					*pNewNode = neighbor;
					_openlist.Push(pNewNode);
				}
			}

			_closelist.Push(pCurNode);
		}

		if (_openlist.Empty())
		{
			//fprintf(stdout, "gastar::nodes_touched\t= %d\n", _nodes_touched);
			//fprintf(stdout, "gastar::nodes_expanded\t= %d\n", _nodes_expanded);
			//fprintf(stdout, "gastar::search failed, no path, start=(%d,%d), goal=(%d,%d)\n\n", start->GetCoord().x, start->GetCoord().z, goal->GetCoord().x, goal->GetCoord().z);
			return false;
		}

		if (!BuildPath(pCurNode, abs_path))
		{
			return false;
		}

		//fprintf(stdout, "gastar ok, start=(%d,%d), goal=(%d,%d), touched:%d, expanded:%d\n", start->GetCoord().x, start->GetCoord().z, goal->GetCoord().x, goal->GetCoord().z, _nodes_touched, _nodes_expanded);

		//fprintf(stdout, "gastar::nodes_touched\t= %d\n", _nodes_touched);
		//fprintf(stdout, "gastar::nodes_expanded\t= %d\n", _nodes_expanded);
		//fprintf(stdout, "gastar::search succ, start=(%d,%d), goal=(%d,%d)\n", start->GetCoord().x, start->GetCoord().z, goal->GetCoord().x, goal->GetCoord().z);

		//clock.End();
		//clock.ElapseTime();
		//fprintf(stdout, "\n");

		return true;
	}

	int32_t GetNodesExpanded() const { return _nodes_expanded; }

protected:
	bool BuildPath(const ABSNode* dest, ABSPath& abs_path) const
	{
		if (!dest)
		{
			return false;
		}
	
		const ABSNode* pNode = dest;
		ABSPath tmp_path;
		for (;;)
		{
			const GEDGE* pEdge = pNode->GetEdge();
			if (pNode && pEdge)
			{
				tmp_path.push_back(pEdge);
				pNode = pNode->GetParent();
			}
			else
			{
				break;
			}
		}
	
		abs_path.clear();
		int i = tmp_path.size() - 1;
		while (i >= 0)
		{
			abs_path.push_back(tmp_path[i]);
			-- i;
		}
		return true;
	}
};

};

#endif // __PF_GASTAR_H__
