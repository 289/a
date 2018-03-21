#ifndef __PF_BITMAP_H__
#define __PF_BITMAP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>

#include "point.h"
#include "pf_common.h"
#include "utility.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

/**
 * @class BitMap--origin walk map
 * @brief 原始通路图，由0和1的序列组成
 * @brief 本通路图内包含1个或多个相互独立的局部通路图，彼此之间不连通.
 */
class Buffer;
class BitMap
{
protected:
private:
  typedef stl_map<int, int> DynMapRefDict; // 动态掩码坐标索引->引用计数的字典
	int32_t _map_id;
	int32_t _map_length;  // how many grids in one row
	int32_t _map_width;   // how many grids in one col
	float   _grid_size;   // min unit of walkmap, the length of side
	int32_t _buf_len;     // the bytes of bitmap's memory
	unsigned char* _pBuf; // bitmap,1:passable,0:can't passable
	PF2DVECTOR _origin;   // the coordinate of walkmap-centre in game-world
	stl_set<int> _dyn_map; // record dynamic movemap, record the index of grid which is set to be un-passable
  DynMapRefDict _dyn_ref; // 
#ifdef _DEBUG
	std::vector<char> _buf;// test
#endif // #ifdef _DEBUG

public:	
	BitMap() :
		_map_id(0),
		_map_length(0),
		_map_width(0),
		_grid_size(0.0f),
		_buf_len(0),
		_pBuf(NULL),
		_origin(0.0f,0.0f)
	{
	}

	~BitMap()
	{
		if (_pBuf)
		{
			free(_pBuf);
			_pBuf = NULL;
			_buf_len = 0;
		}

		_map_id       = 0;
		_map_length   = 0;
		_map_width    = 0;
		_grid_size    = 0;
		_buf_len      = 0;
		_pBuf         = 0;

		_dyn_map.clear();
    _dyn_ref.clear();
#ifdef _DEBUG
		_buf.clear();
#endif // #ifdef _DEBUG
	}

	friend class MapDataGenerator;

	bool Load(shared::net::Buffer& buffer)
	{
		_map_id       = buffer.ReadInt32();
		_map_length   = buffer.ReadInt32();
		_map_width    = buffer.ReadInt32();
		_grid_size    = ReadFloatFromBuffer(buffer);
		_origin.x     = ReadFloatFromBuffer(buffer);
		_origin.z     = ReadFloatFromBuffer(buffer);
		_buf_len      = buffer.ReadInt32();

		unsigned char* __buf = NULL;
		size_t __len         = 0;
		size_t __grid_count  = 0;

		__grid_count = _map_length * _map_width;
		__len = __grid_count >> 3;
		if (__grid_count & 0x07)
		{
			__len += 1;
		}

		if (__len != (size_t)(_buf_len))
		{
			return false;
		}

		__buf = (unsigned char*)malloc(__len);
		if (!__buf)
		{
			return false;
		}

		if (buffer.ReadableBytes() < __len)
		{
			free(__buf);
			return false;
		}

		::memset(__buf, 0, __len);
		::memcpy(__buf, buffer.peek(), __len);
		buffer.Retrieve(__len);

		_pBuf    = __buf;
		_buf_len = __len;

#ifdef _DEBUG
    // test
		_buf.resize(__grid_count);
		for (size_t i = 0; i < __grid_count; ++ i)
			_buf[i] = TestBit(i) ? '1' : '0';

		//test
		//dump();
#endif // #ifdef _DEBUG
		return true;
	}

	bool Save(shared::net::Buffer& buffer)
	{
		buffer.AppendInt32(_map_id);
		buffer.AppendInt32(_map_length);
		buffer.AppendInt32(_map_width);
		buffer.Append(&_grid_size, sizeof(float));
		buffer.Append(&(_origin.x), sizeof(float));
		buffer.Append(&(_origin.z), sizeof(float));
		buffer.AppendInt32(_buf_len);
		buffer.Append(_pBuf, _buf_len);
		return true;
	}

	bool IsValidPos(const PointI& pt) const
	{
		return (pt.x >= 0 && pt.x < _map_length) && (pt.z >= 0 && pt.z < _map_width);
	}

	bool IsPassable(const PointF& pt) const
	{
		return IsPassable(POINTF_2_POINTI(pt));
	}

	bool IsPassable(const PointI& pt) const
	{
		if (pt.x < 0 || pt.x >= _map_length || pt.z < 0 || pt.z >= _map_width)
			return false;
		return TestBit(TransCoordToIndex(pt));
	}

  bool SetDynPassable(const PointI* pos, int count, bool passable)
  {
    typedef std::pair<DynMapRefDict::iterator, bool> IndexRefPair;
    if( ! passable)
    {
      for(int i = 0; i < count; i++)
      {
        size_t index = TransCoordToIndex(pos[i]);

        IndexRefPair result = _dyn_ref.insert(stl_make_pair(index, 1));
        if(result.second) {
          SetBitUnsafe(index, passable);
        }
        else {
          ++result.first->second;
        }
      }
    }
    else
    {
      for(int i = 0; i < count; i++)
      {
        size_t index = TransCoordToIndex(pos[i]);

        DynMapRefDict::iterator result = _dyn_ref.find(index);
        if(result != _dyn_ref.end()) {
          if((--result->second) == 0) {
            SetBitUnsafe(index, passable);
            _dyn_ref.erase(result);
          }
        }
      }
    }
    return true;
  }

	bool SetDynPassable(const PointI& pos, bool passable)
	{
		size_t index = TransCoordToIndex(pos);

		if (!passable)
		{
			if (!TestBit(index)) {
				return false;
      }

			if (_dyn_map.find(index) != _dyn_map.end()) {
				return false;
      }

			_dyn_map.insert(index);
			SetBit(index, false);
			return true;
		}
		else
		{
			if (TestBit(index)) {
				return false;
      }

      if (_dyn_map.find(index) == _dyn_map.end()) {
        return false;
      }

			_dyn_map.erase(index);
			SetBit(index, true);
			return true;
		}

		return false;
	}

	bool TestBit(size_t index) const
	{
		if (index < 0 || index >= (size_t)(_map_width * _map_length))
			return false;
		return _pBuf[index >> 3] & (unsigned char)(1 << (7 - (index & 0x07)));
	}

  void SetBitUnsafe(size_t index, bool passable) // 不检查index合法性
  {
    if (passable) {
      _pBuf[index >> 3] |= (unsigned char)(1 << (7 - (index & 0x07)));
    }
    else {
      _pBuf[index >> 3] &= ~(unsigned char)(1 << (7 - (index & 0x07)));
    }
  }

	void SetBit(size_t index, bool passable)
	{
		if (index < 0 || index >= (size_t)(_map_width * _map_length)) {
			return;
    }

    SetBitUnsafe(index, passable);
	}

	PointF TransWorld2Map(const PF2DVECTOR& vpos) const
	{
		float tmp_X = (-_origin.x + vpos.x) / _grid_size;
		float tmp_Z = (-_origin.z + vpos.z) / _grid_size;
		return PointF(tmp_X, tmp_Z);
	}
	
	PF2DVECTOR TransMap2World(const PointI& pt) const
	{
		return TransMap2World(POINTI_2_POINTF(pt));
	}
	
	PF2DVECTOR TransMap2World(const PointF& pt) const
	{
		float tmp_X = _origin.x + pt.x * _grid_size;
		float tmp_Z = _origin.z + pt.z * _grid_size;
		return PF2DVECTOR(tmp_X, tmp_Z);
	}

	int32_t GetMapID() const       { return _map_id; }
	int32_t GetMapWidth() const    { return _map_width; }
	int32_t GetMapLength() const   { return _map_length; }
	float   GetGridSize() const    { return _grid_size; }

#ifdef _DEBUG
	//test
	void Clear()
	{
		for (size_t i = 0; i < _buf.size(); ++ i)
		{
			_buf[i] = ' ';
		}
	}

	//test
	void SetChar(const PointI& pt, char c)
	{
		if (pt.x < 0 || pt.x >= _map_length ||
			pt.z < 0 || pt.z >= _map_width)
		{
			assert(false);
			return;
		}

		size_t index = TransCoordToIndex(pt);
		if (TestBit(index))
		{
			_buf[index] = c;
		}
	}

	//test
	void SetExpandBit(const PointI& pt)
	{
		if (pt.x < 0 || pt.x >= _map_length ||
			pt.z < 0 || pt.z >= _map_width)
		{
			assert(false);
			return;
		}

		size_t index = TransCoordToIndex(pt);
		if (TestBit(index))
		{
			_buf[index] = '*';
		}
	}

	//test
	void dump(char c1='1', char c2='0')
	{
		fprintf(stdout, "mapid: %d\n", _map_id);
		for (int i = _map_width - 1; i >= 0; -- i)
		{
			fprintf(stdout, "%3d  ", i);
			for (int j = 0; j < _map_length; ++ j)
			{
				fprintf(stdout, "%c", _buf[i*_map_length+j]);
			}

			fprintf(stdout, "\n");
		}

		fprintf(stdout, "\n    ");
		for (int i = 0; i < _map_length; ++ i)
			fprintf(stdout, "%d", i%10);
		fprintf(stdout, "\n");

		int32_t __grid_count = _map_length * _map_width;
		for (int i = 0; i < __grid_count; ++ i)
		{
			_buf[i] = TestBit(i) ? c1 : c2;
		}
	}
#endif // #ifdef _DEBUG
private:
	size_t TransCoordToIndex(const PointI& pt) const
	{
		size_t index = pt.z * _map_length + pt.x;
		return index;
	}
};

}; // namespace pathfinder

#endif // __PF_BITMAP_H__
