#ifndef __PF_MOVE_MAP_H__
#define __PF_MOVE_MAP_H__

#include <vector>
#include <stdint.h>
#include "point.h"
#include "cluster.h"

namespace shared {
namespace net {
class Buffer;
}; // namespace shared
}; // namespace net

namespace pathfinder
{

/**
 * 地图由一张或多张小地图拼接而成，每张小地图称为一个"区域"或"簇"。
 * 簇的形状是规则的，为正方形或者长方形。
 *
 *
 * ^
 * |
 * |-------------------------------------
 * |                                    |
 * |          .......                   |
 * |                                    |
 * |------------------------------------|
 * |          |             |           |
 * | Cluster1 |   .......   | Cluster_N |
 * |          |             |           |
 * |------------------------------------------------->
 * |(0,0)
 * |
 *
 */

class BitMap;
class DGraph;
class Cluster;
class PathTrack;

typedef std::vector<Cluster*> ClusterVec;

class MoveMap
{
private:
	int32_t _map_length;      // 地图长度(水平方向格子个数)
	int32_t _map_width;       // 地图宽度(垂直方向格子个数)

	BitMap*    _bmap;         // 全局通路图
	DGraph*    _graph;        // 本地图的传送图
	ClusterVec _clusters;     // 簇列表
	bool       _active;       // 标记是否在被使用(作用类似锁)

public:
	MoveMap();
	virtual ~MoveMap();

	friend class MapDataGenerator;

	bool Load(shared::net::Buffer& buffer);
	void Save(shared::net::Buffer& buffer);

	bool IsValidPos(const PointI& pt) const;
	bool IsPassable(const PointI& pt) const;
	bool IsPassable(const PointF& pt) const;
	bool InSameArea(const PointF& pt, const PointF& pt2) const;

	PointF    TransWorld2Map(const PF2DVECTOR& vpos) const;
	PF2DVECTOR TransMap2World(const PointI& pt) const;
	PF2DVECTOR TransMap2World(const PointF& pt) const;

	bool LineTo(const PointI& from, const PointI& to) const;
	bool LineTo(const PointF& from, const PointF& to) const;
	bool LinePath(const PointI& from, const PointI& to, Path& path) const;

	bool LineTo2(const PointI& from, const PointI& to) const;
	bool LineTo2(const PointF& from, const PointF& to) const;
	bool LinePath2(const PointI& from, PointI& to, Path& path) const;
	const Cluster* LocateCluster(const PointI& pt) const;

	int32_t  GetMapID();
	DGraph*  GetGraph();
	BitMap*  GetBitMap();
	Cluster* GetCluster(size_t index);
	float    GetGridSize();

	void SetActive();
	void ClrActive();
	bool TestActive() const;
};

///
/// inline func
///

inline void MoveMap::SetActive()
{
	_active = true;
}

inline void MoveMap::ClrActive()
{
	_active = false;
}

inline bool MoveMap::TestActive() const
{
	return _active;
}

};

#endif // __PF_MOVE_MAP_F__
