#include <assert.h>
#include <algorithm>
#include "graph.h"
#include "utility.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/*********************************Graph************************************/
/*********************************Graph************************************/
/*********************************Graph************************************/
/*********************************Graph************************************/

Graph::Graph()
{
}

Graph::~Graph()
{
	for (size_t i = 0; i < _nodes.size(); ++ i)
	{
		delete _nodes[i];
		_nodes[i] = NULL;
	}

	for (size_t i = 0; i < _edges.size(); ++ i)
	{
		delete _edges[i];
		_edges[i] = NULL;
	}

	_nodes.clear();
	_edges.clear();
}

bool Graph::Load(shared::net::Buffer& buffer)
{
	int32_t node_count = buffer.ReadInt32();
	for (int i = 0; i < node_count; ++ i)
	{
		int16_t coordX, coordZ;
		coordX = buffer.ReadInt16();
		coordZ = buffer.ReadInt16();

		GNode* node = new GNode(coordX, coordZ);
		if (!node) return false;
		AddNode(node);
	}

	int32_t edge_count = buffer.ReadInt32();
	for (int i = 0; i < edge_count; ++ i)
	{
		int16_t from = buffer.ReadInt16();
		int16_t to   = buffer.ReadInt16();
		float weight = ReadFloatFromBuffer(buffer);

		GEdge* edge = new GEdge(from, to, weight);
		if (!edge) return false;
		AddEdge(edge);
	}

	return true;
}

void Graph::Save(shared::net::Buffer& buffer)
{
	int32_t node_count = _nodes.size();
	buffer.AppendInt32(node_count);
	for (int i = 0; i < node_count; ++ i)
	{
		GNode* node = _nodes[i];
		PointI pt   = node->GetCoord();
		buffer.AppendInt16((int16_t)(pt.x));
		buffer.AppendInt16((int16_t)(pt.z));
	}

	int32_t edge_count = _edges.size();
	buffer.AppendInt32(edge_count);
	for (int i = 0; i < edge_count; ++i)
	{
		const GEdge* edge = _edges[i];
		buffer.AppendInt16(edge->GetFrom());
		buffer.AppendInt16(edge->GetTo());
		float weight = edge->GetWeight();
		buffer.Append(&weight, sizeof(float));
	}
}

template<class T>
struct Finder
{
	int index;
	Finder(int idx): index(idx) {}
	bool operator() (const T* obj) const
	{
		return obj->GetIndex() == index;
	}
};

bool Graph::AddNode(GNode* node)
{
	if (!node)
	{
		return false;
	}

	node->_node_idx = _nodes.size();
	_nodes.push_back(node);
	return true;
}

bool Graph::RmvNode(GNode* node)
{
	/**
	 * @brief 节点INDEX依赖于节点在容器中的位置,这里约定删除节点从最后一个节点开始删除
	 */
	if (!node)
	{
		return false;
	}

	while (!node->GetEdges().empty())
	{
		//注意：
		//迭代过程中node->GetEdges()大小在变化
		RmvEdge(node->GetEdges().back());
	}

	assert(node == _nodes.back());
	if (node == _nodes.back())
	{
		_nodes.pop_back();
		//delete node;
		return true;
	}

	return true;
}

bool Graph::AddEdge(GEdge* edge)
{
	if (!edge)
	{
		return false;
	}

	int32_t from = edge->GetFrom();
	int32_t to = edge->GetTo();
	_nodes[from]->AddEdge(edge);
	_nodes[to]->AddEdge(edge);

	edge->_edge_idx = _edges.size();
	_edges.push_back(edge);
	return true;
}

bool Graph::RmvEdge(GEdge* edge)
{
	if (!edge)
	{
		return false;
	}

	int32_t from = edge->GetFrom();
	int32_t to = edge->GetTo();
	GetNode(from)->RmvEdge(edge);
	GetNode(to)->RmvEdge(edge);

	if (edge == _edges.back())
	{
		_edges.pop_back();
		delete edge;
		return true;
	}

	_edges[edge->GetIndex()] = _edges.back();
	_edges[edge->GetIndex()]->SetIndex(edge->GetIndex());
	_edges.pop_back();

	delete edge;
	return true;
}

GNode* Graph::GetNode(int index) const
{
	if (index < 0 || index >= (int)(_nodes.size()))
	{
		assert(false && "NODE索引越界了!!!");
		return NULL;
	}
	return _nodes[index];
}

GEdge* Graph::GetEdge(int index) const
{
	if (index < 0 || index >= (int)_edges.size())
	{
		assert(false && "EDGE编号越界了!!!");
		return NULL;
	}
	return _edges[index];
}

const GNodeVec& Graph::GetAllNodes() const
{
	return _nodes;
}

const GEdgeVec& Graph::GetAllEdges() const
{
	return _edges;
}

void Graph::dump()
{
}

/*********************************GEdge************************************/
/*********************************GEdge************************************/
/*********************************GEdge************************************/
/*********************************GEdge************************************/

GEdge::GEdge(int32_t end1, int32_t end2, float w)
	:
	_edge_idx(-1)
{
	_end[0]     = end1;
	_end[1]     = end2;
	_weight     = w;
}

GEdge::~GEdge()
{
	_end[0] = 0;
	_end[1] = 0;
	_weight = -1;
	_edge_idx = 0;
}

void GEdge::SetIndex(int32_t index)
{
	_edge_idx = index;
}

int32_t GEdge::GetIndex() const
{
	return _edge_idx;
}

int32_t GEdge::GetFrom() const
{
	return _end[0];
}

int32_t GEdge::GetTo() const
{
	return _end[1];
}


/*********************************GNode************************************/
/*********************************GNode************************************/
/*********************************GNode************************************/
/*********************************GNode************************************/

GNode::GNode():
	_x(0),
	_z(0),
	_node_idx(0)
{
}

GNode::GNode(short x, short z):
	_x(x),
	_z(z),
	_node_idx(-1)
{
}

GNode::GNode(const GNode& node):
	_x(node._x),
	_z(node._z),
	_node_idx(node._node_idx),
	_edges(node._edges)
{
}


GNode::~GNode()
{
	_x = 0;
	_z = 0;
	_node_idx = 0;
	_edges.clear();
}

void GNode::AddEdge(GEdge* e)
{
	if (!e) return;

	GEdgeVec::const_iterator it = std::find(_edges.begin(), _edges.end(), e);
	assert(it == _edges.end());
	_edges.push_back(e);
}

void GNode::RmvEdge(GEdge* e)
{
	if (!e) return;
	if (!e->IsEdgeEnd(_node_idx))
	{
		return;
	}

	size_t i = 0;
	for (; i < _edges.size(); ++ i)
	{
		if (_edges[i] == e)
			break;
	}

	assert(i < _edges.size());
	_edges[i] = _edges.back();
	_edges.pop_back();
}

const GEdgeVec& GNode::GetNeighbours() const
{
	return _edges;
}

};
