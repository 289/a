#ifndef __PF_GRAPH_H__
#define __PF_GRAPH_H__

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include "point.h"
#include "types.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace net
}; // namespace shared

namespace pathfinder
{

class GNode;
class GEdge;
class Graph;

typedef std::vector<GNode*> GNodeVec;
typedef std::vector<GEdge*> GEdgeVec;

/**
 * @class: Graph
 * @brief: unoriented graph--无向图
 * @brief: Graph由cluster组成,cluster为正方形,长度为_cluster_size
 *         cluster本身可以由尺寸更小的cluster组成,即cluster可以嵌套
 *         最里面的cluster级别记为level-0,往外依次为level-1...leveln
 */

class Graph
{
private:
	GNodeVec _nodes;
	GEdgeVec _edges;

public:
	Graph();
	~Graph();

	bool Load(shared::net::Buffer& buffer);
	void Save(shared::net::Buffer& buffer);

	bool AddNode(GNode* n);
	bool RmvNode(GNode* n);
	bool AddEdge(GEdge* e);
	bool RmvEdge(GEdge* e);

	GNode* GetNode(int index) const;
	GEdge* GetEdge(int index) const;

	const GNodeVec& GetAllNodes() const;
	const GEdgeVec& GetAllEdges() const;

	void dump();
};


class GEdge
{
private:
	float   _weight;   // the weight of the edge
	int32_t _end[2];   // two end of the edge
	int32_t _edge_idx; // edge index in graph

public:
	GEdge(int32_t end1, int32_t end2, float w);
	~GEdge();

	friend class Graph;

	void    SetIndex(int32_t index);
	bool    IsEdgeEnd(int32_t index) const;
	float   GetWeight() const;
	int32_t GetIndex() const;
	int32_t GetFrom() const;
	int32_t GetTo() const;
};


class GNode
{
private:
	short _x, _z;      // coordinate of the node
	int32_t _node_idx; // node index in the graph
	GEdgeVec _edges;   // adjacent edges with this node

public:
	GNode();
	GNode(short x, short z);
	GNode(const GNode& node);
	~GNode();

	friend class Graph;
	friend class GEdge;

	bool operator == (const GNode& rhs) const
	{
		return _x == rhs._x && _z == rhs._z;
	}

	int32_t GetIndex() const;
	const GEdgeVec& GetEdges() const;
	void SetCoord(const PointI& pt);
	PointI GetCoord() const;
	const GEdgeVec& GetNeighbours() const;

	void AddEdge(GEdge* e);
	void RmvEdge(GEdge* e);
};

typedef std::vector<const GEdge*> AbsPath;


///
/// inline func
///

inline bool GEdge::IsEdgeEnd(int32_t index) const
{
	return _end[0] == index || _end[1] == index;
}

inline float GEdge::GetWeight() const
{
	return _weight;
}

inline int32_t GNode::GetIndex() const
{
	return _node_idx;
}

inline const GEdgeVec& GNode::GetEdges() const
{
	return _edges;
}

inline void GNode::SetCoord(const PointI& pt)
{
	_x = pt.x;
	_z = pt.z;
}

inline PointI GNode::GetCoord() const
{
	return PointI(_x, _z);
}

};

#endif // __PF_GRAPH_H__
