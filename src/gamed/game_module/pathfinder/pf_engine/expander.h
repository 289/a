#ifndef __PF_EXPANDER_H__
#define __PF_EXPANDER_H__

#include <stdio.h>
#include <list>
#include <deque>
#include "point.h"
#include "types.h"
#include "base_astar.h"
#include "bit_map.h"
#include "pf_common.h"

namespace pathfinder
{

/**
 * @class Expander
 * @brief 寻找目标点附近的可达点
 */
class BitMap;
class Expander
{
protected:
	struct Node
	{
		PointI pt;
		Node(PointI __pt): pt(__pt) {}
		const PointI& GetCoord() const {return pt;}
		int32_t GetOwner() const {return 0;}
	};

private:
	std::deque<Node*> _openlist; //等待遍历的节点列表(全部是不可达点)
	CloseHash<Node> _openlist2; //辅助开放列表(加快主开放列表的查询)
	CloseHash<Node> _closelist; //已经遍历的节点列表(全部是不可达点)

public:
	Expander()
	{
	}
	~Expander()
	{
		_openlist.clear();
		_openlist2.Clear();
		_closelist.Clear();
	}

	bool Search(const BitMap* bitmap, const PointI& pos/*non-walkable*/, PointI& newpos/*walkable*/)
	{
		if (!bitmap || !bitmap->IsValidPos(pos))
		{
			return false;
		}
	
		/*
		if (bitmap->IsPassable(pos))
		{
			newpos = pos;
			return true;
		}
		*/

		Node* pNode = new Node(pos);
		if (!pNode)
		{
			return false;
		}

		_openlist.push_back(pNode);
		_openlist2.Push(pNode);

		//循环处理OPEN列表中的节点
		while (!_openlist.empty())
		{
			Node* __node = _openlist.front();
			for (size_t i = 0; i < PF_NEIGHBOR_COUNT; ++ i)
			{
				PointI __pt = __node->GetCoord();
				__pt.Offset(PF2D_NeighborDist[i*2], PF2D_NeighborDist[i*2+1]);
				if (__pt != pos && bitmap->IsPassable(__pt))
				{
					newpos = __pt;
					return true;
				}
				else if (bitmap->IsValidPos(__pt))
				{
					Node next_node(__pt);
					if (!_closelist.Find(next_node) && !_openlist2.Find(next_node))
					{
						Node* pNextNode = new Node(__pt);
						if (!pNextNode)
						{
							return false;
						}
						_openlist.push_back(pNextNode);
						_openlist2.Push(pNextNode);
					}
					else
					{
						//已经存在于“已访问列表”
						//无需再次插入
					}
				}
				else
				{
					//超出地图范围的无效点
				}
			}

			//删除刚才访问的节点
			_openlist.pop_front();
			_openlist2.Remove(__node);
			_closelist.Push(__node);
		};
	
		return false;
	}

private:
	void dump()
	{
		fprintf(stdout, "开放列表：\n");
		std::deque<Node*>::iterator it = _openlist.begin();
		for (; it != _openlist.end(); ++ it)
		{
			Node* node= *it;
			PointI pt = node->GetCoord();
			fprintf(stdout, "(%d,%d),", pt.x, pt.z);
		}

		fprintf(stdout, "\n已访问列表：\n");
		_closelist.Dump();
		fprintf(stdout, "\n");
	}
};

};

#endif // __PF_EXPANDER_H__
