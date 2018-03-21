#ifndef __PF_CLUSTER_H__
#define __PF_CLUSTER_H__

#include <vector>
#include "point.h"
#include "types.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/**
 * @class Cluster
 * @brief 簇是小地图，多个簇相互连接成一张大地图，簇和簇之间通过传送点连接。
 * @brief 玩家跨越簇时，类似跨地图操作，需要通过传送点来实现。
 *
 *       length
 *  ------------------
 *  |                |
 *  |                | width
 *  |                |
 *  ------------------
 *  _base_pos(left-down-corner)
 *
 */

class Cluster
{
protected:
	typedef int16_t NodeNum;
	typedef std::vector<NodeNum> NodeVec;

private:
	PointI  _base_pos; // the coordinate of left down corner
	int32_t _length;   // the length of cluster(unit is base grid)
	int32_t _width;    // the width of cluster(unit is base grid)
	int16_t _index;    // the index of cluster self
	NodeVec _in_list;  // the list of entrances
	NodeVec _out_list; // the list of exits

public:
	Cluster():
		_length(0),
		_width(0),
		_index(0)
	{
	}

	~Cluster()
	{
		_base_pos = PointI(-1,-1);
		_length = 0;
		_width  = 0;
		_index  = 0;
		_in_list.clear();
		_out_list.clear();
	}
	
	friend class MapDataGenerator;

public:
	void Load(shared::net::Buffer& buffer)
	{
		_base_pos.x = buffer.ReadInt16();
		_base_pos.z = buffer.ReadInt16();
		_length     = buffer.ReadInt32();
		_width      = buffer.ReadInt32();

		int32_t __count = buffer.ReadInt32();
		if (__count > 0)
		{
			_in_list.resize(__count);
			for (int i = 0; i < __count; ++ i)
			{
				_in_list[i] = buffer.ReadInt16();
			}
		}

		__count = buffer.ReadInt32();
		if (__count > 0)
		{
			_out_list.resize(__count);
			for (int i = 0; i < __count; ++ i)
			{
				_out_list[i] = buffer.ReadInt16();
			}
		}
	}

	void Save(shared::net::Buffer& buffer)
	{
		buffer.AppendInt16(_base_pos.x);
		buffer.AppendInt16(_base_pos.z);
		buffer.AppendInt32(_length);
		buffer.AppendInt32(_width);

		int32_t __count = _in_list.size();
		buffer.AppendInt32(__count);
		for (int i = 0; i < __count; ++ i)
		{
			buffer.AppendInt16(_in_list[i]);
		}

		__count = _out_list.size();
		buffer.AppendInt32(__count);
		for (int i = 0; i < __count; ++ i)
		{
			buffer.AppendInt16(_out_list[i]);
		}
	}

	bool TestRange(const PointI& pt) const
	{
		if (pt.x < _base_pos.x || pt.z < _base_pos.z)
			return false;

		PointI offset = pt - _base_pos;
		return offset.x <= _length && offset.z <= _width;
	}

	void SetIndex(int16_t index)
	{
		_index = index;
	}

	int16_t GetIndex() const
	{
		return _index;
	}

	const NodeVec& GetInList() const
	{
		return _in_list;
	}

	const NodeVec& GetOutList() const
	{
		return _out_list;
	}
};

};

#endif // __PF_CLUSTER_H__
