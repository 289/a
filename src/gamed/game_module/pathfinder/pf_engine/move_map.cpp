#include <math.h>
#include <assert.h>
#include <algorithm>

#include "game_stl.h"
#include "move_map.h"
#include "bit_map.h"
#include "dgraph.h"
#include "cluster.h"
#include "path_track.h"
#include "pf_common.h"

namespace pathfinder
{

MoveMap::MoveMap():
	_map_length(0),
	_map_width(0),
	_bmap(NULL),
	_graph(NULL),
	_active(false)
{
}

MoveMap::~MoveMap()
{
	if (_bmap)
	{
		delete _bmap;
		_bmap = NULL;
	}

	if (_graph)
	{
		delete _graph;
		_graph = NULL;
	}

	for (size_t i = 0; i < _clusters.size(); ++ i)
	{
		delete _clusters[i];
		_clusters[i] = NULL;
	}

	_clusters.clear();

	_map_length    = 0;
	_map_width     = 0;
	_active        = false;
}

int32_t MoveMap::GetMapID()
{
	return _bmap->GetMapID();
}

BitMap* MoveMap::GetBitMap()
{
	return _bmap;
}

DGraph* MoveMap::GetGraph()
{
	return _graph;
}

Cluster* MoveMap::GetCluster(size_t index)
{
	if (index < 0 || index >= _clusters.size())
		return NULL;
	return _clusters[index];
}

float MoveMap::GetGridSize()
{
	return _bmap->GetGridSize();
}

bool MoveMap::Load(shared::net::Buffer& buffer)
{
	_bmap = new BitMap();
	if (!_bmap || !_bmap->Load(buffer))
	{
		return false;
	}

	//init the size of movemap
	_map_length   = _bmap->GetMapLength();
	_map_width    = _bmap->GetMapWidth();

	if (buffer.ReadableBytes() <= 0)
	{
		//地图未划分簇
		return true;
	}

	_graph = new DGraph();
	if (!_graph || !_graph->Load(buffer))
	{
		return false;
	}

	//加载族
	int32_t cluster_count = buffer.ReadInt32();
	_clusters.resize(cluster_count);
	for (int i = 0; i < cluster_count; ++ i)
	{
		Cluster* cluster = new Cluster();
		if (!cluster) return false;

		cluster->SetIndex(i);
		cluster->Load(buffer);
		_clusters[i] = cluster;
	}

	assert(buffer.ReadableBytes() == 0);

	return true;
}

void MoveMap::Save(shared::net::Buffer& buffer)
{
	_bmap->Save(buffer);
	if (!_graph)
	{
		return;
	}

	_graph->Save(buffer);

	int32_t cluster_count = _clusters.size();
	buffer.AppendInt32(cluster_count);
	for (int i = 0; i < cluster_count; ++ i)
	{
		Cluster* cluster = _clusters[i];
		cluster->Save(buffer);
	}
}

bool MoveMap::IsValidPos(const PointI& pt) const
{
	if (pt.x < 0 || pt.x >= _map_length ||
		pt.z < 0 || pt.z >= _map_width)
	{
		return false;
	}
	return true;
}

bool MoveMap::IsPassable(const PointI& pt) const
{
	return _bmap->IsPassable(pt);
}

bool MoveMap::IsPassable(const PointF& pt) const
{
	return _bmap->IsPassable(pt);
}

bool MoveMap::InSameArea(const PointF& pt1, const PointF& pt2) const
{
	const Cluster* cluster1 = LocateCluster(POINTF_2_POINTI(pt1));
	const Cluster* cluster2 = LocateCluster(POINTF_2_POINTI(pt2));
	return cluster1 == cluster2;
}

PointF MoveMap::TransWorld2Map(const PF2DVECTOR& vpos) const
{
	return _bmap->TransWorld2Map(vpos);
}

PF2DVECTOR MoveMap::TransMap2World(const PointI& pt) const
{
	return _bmap->TransMap2World(pt);
}

PF2DVECTOR MoveMap::TransMap2World(const PointF& pt) const
{
	return _bmap->TransMap2World(pt);
}

bool MoveMap::LineTo(const PointF& from, const PointF& to) const
{
	return LineTo(POINTF_2_POINTI(from), POINTF_2_POINTI(to));
}

bool MoveMap::LineTo(const PointI& from, const PointI& to) const
{
	if (from == to)
	{
		return true;
	}

	PointF __from = POINTI_2_POINTF(from);
	PointF __to   = POINTI_2_POINTF(to);

	PointF dir(__to - __from);
	float len = dir.Magnitude();
	dir.Normalization();

	int count = 0;
	PointI cur = from;
	while (((float)count < len) && (cur != to))
	{
		++count;
		cur.x = (int)(from.x + dir.x * count);
		cur.z = (int)(from.z + dir.z * count);
		if (!IsPassable(cur))
		{
			return false;
		}
	}
	return true;
}

bool MoveMap::LinePath(const PointI& from, const PointI& to, Path& path) const
{
	if (from == to)
	{
		path.push_back(from);
		return true;
	}

	PointF __from = POINTI_2_POINTF(from);
	PointF __to   = POINTI_2_POINTF(to);

	PointF dir(__to - __from);
	float len = dir.Magnitude();
	dir.Normalization();

	int count = 0;
	PointI cur = from;
	while (((float)count < len) && (cur != to))
	{
		++count;
		cur.x = (int)(from.x + dir.x * count);
		cur.z = (int)(from.z + dir.z * count);
		if (IsPassable(cur))
		{
			path.push_back(cur);
		}
		else
		{
			assert(false);
			return false;
		}
	}

	return true;
}

bool MoveMap::LineTo2(const PointF& from, const PointF& to) const
{
	return LineTo2(POINTF_2_POINTI(from), POINTF_2_POINTI(to));
}

bool MoveMap::LineTo2(const PointI& from, const PointI& to) const
{
	if (from == to)
	{
		return true;
	}

	short deltaX = to.x - from.x;
	short deltaZ = to.z - from.z;

	short stepX = 0;
	short stepZ = 0;

	if (deltaX < 0) stepX = -1; else stepX = 1;
	if (deltaZ < 0) stepZ = -1; else stepZ = 1;

	deltaX = abs(deltaX);
	deltaZ = abs(deltaZ);

	short nextX = from.x;
	short nextZ = from.z;
	short goalX = to.x;
	short goalZ = to.z;

	if (deltaZ > deltaX)
	{
		short fraction = 2 * deltaX - deltaZ;
		while (nextZ != goalZ)
		{
			if (fraction >= 0)
			{
				nextX += stepX;
				fraction += (deltaX << 1) - (deltaZ << 1);
			}
			else
			{
				fraction += (deltaX << 1);
			}

			nextZ += stepZ;
			PointI __tmp(nextX, nextZ);
			if (!IsPassable(__tmp))
			{
				return false;
			}
		};
	}
	else
	{
		short fraction = 2 * deltaZ - deltaX;
		while (nextX != goalX)
		{
			if (fraction >= 0)
			{
				nextZ += stepZ;
				fraction += (deltaZ << 1) - (deltaX << 1);
			}
			else
			{
				fraction += (deltaZ << 1);
			}

			nextX += stepX;
			PointI __tmp(nextX, nextZ);
			if (!IsPassable(__tmp))
			{
				return false;
			}
		};
	}
	return true;
}

bool MoveMap::LinePath2(const PointI& from, PointI& to, Path& path) const
{
	if (from == to)
	{
		path.push_back(from);
		return true;
	}

	short deltaX = to.x - from.x;
	short deltaZ = to.z - from.z;

	short stepX = 0;
	short stepZ = 0;

	if (deltaX < 0) stepX = -1; else stepX = 1;
	if (deltaZ < 0) stepZ = -1; else stepZ = 1;

	deltaX = abs(deltaX);
	deltaZ = abs(deltaZ);

	path.push_back(from);

	short nextX = from.x;
	short nextZ = from.z;
	short goalX = to.x;
	short goalZ = to.z;

	if (deltaZ > deltaX)
	{
		short fraction = 2 * deltaX - deltaZ;
		while (nextZ != goalZ)
		{
			if (fraction >= 0)
			{
				nextX += stepX;
				fraction += (deltaX << 1) - (deltaZ << 1);
			}
			else
			{
				fraction += (deltaX << 1);
			}

			nextZ += stepZ;
			PointI __tmp(nextX, nextZ);
			if (!IsPassable(__tmp))
			{
				path.clear();
				return false;
			}

			path.push_back(__tmp);
		};
	}
	else
	{
		short fraction = 2 * deltaZ - deltaX;
		while (nextX != goalX)
		{
			if (fraction >= 0)
			{
				nextZ += stepZ;
				fraction += (deltaZ << 1) - (deltaX << 1);
			}
			else
			{
				fraction += (deltaZ << 1);
			}

			nextX += stepX;
			PointI __tmp(nextX, nextZ);
			if (!IsPassable(__tmp))
			{
				path.clear();
				return false;
			}

			path.push_back(__tmp);
		};
	}
	return true;
}

const Cluster* MoveMap::LocateCluster(const PointI& pt) const
{
	for (size_t i = 0; i < _clusters.size(); ++ i)
	{
		if (_clusters[i]->TestRange(pt))
		{
			return _clusters[i];
		}
	}

	assert(false);
	return NULL;
}

template <class T>
struct Finder
{
	int32_t index;
	Finder(int32_t idx): index(idx) {}
	bool operator() (const T* obj) const
	{
		return index == obj->GetIndex();
	}
};

};
