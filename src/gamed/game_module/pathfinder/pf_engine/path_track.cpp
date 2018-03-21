#include "path_track.h"
#include "pf_common.h"
#include "shared/net/buffer.h"

namespace pathfinder
{

PathTrack::PathTrack():_index(-1)
{
}

PathTrack::~PathTrack()
{
	_path.clear();
}

PathTrack::PathTrack(const PathTrack& path)
{
	*this = path;
}

PathTrack& PathTrack::operator = (const PathTrack& path)
{
	if (this == &path)
	{
		return *this;
	}

	_path = path.GetPath();
	return *this;
}

void PathTrack::Init(const Path& path)
{
	assert(_path.empty());
	for (size_t i = 0; i < path.size(); ++ i)
	{
		const PointI& pt = path[i];
		_path.push_back(PointF(float(pt.x), float(pt.z)));
	}
}

void PathTrack::Init(const PATH& path)
{
	assert(_path.empty());
	_path = path;
}

void PathTrack::Clear()
{
	_path.clear();
}

void PathTrack::SetIndex(int32_t index)
{
	_index = index;
}

int32_t PathTrack::GetIndex() const
{
	return _index;
}

const PATH& PathTrack::GetPath() const
{
	return _path;
}

size_t PathTrack::GetLength() const
{
	return _path.size();
}

void PathTrack::GetEndPos(PointF& pos) const
{
	if (_path.size() > 0)
	{
		pos = _path[_path.size()-1];
	}
}

void PathTrack::Load(shared::net::Buffer& buffer)
{
	int32_t count = buffer.ReadInt32();

	_path.clear();
	_path.resize(count);
	for (int i = 0; i < count; ++ i)
	{
		PointF pt;
		pt.x = buffer.ReadInt16();
		pt.z = buffer.ReadInt16();
		_path[i] = pt;
	}
}

void PathTrack::Save(shared::net::Buffer& buffer)
{
	int32_t count = _path.size();
	buffer.AppendInt32(count);
	for (int i = 0; i < count; ++ i)
	{
		PointF& pt = _path[i];
		buffer.AppendInt16((int16_t)(pt.x));
		buffer.AppendInt16((int16_t)(pt.z));
	}
}

bool PathTrack::Append(const Path& path)
{
	PathTrack path_track;
	path_track.Init(path);
	return Append(&path_track);
}

bool PathTrack::Append(const PATH& path)
{
	if (_path.empty())
	{
		_path = path;
		return true;
	}

	if (_path.back().x == path.front().x && _path.back().z == path.front().z)
	{
		//跳过重复点
		for (size_t i = 1; i < path.size(); ++ i)
		{
			_path.push_back(path[i]);
		}
	}
	else if (_path.back().x == path.back().x  && _path.back().z == path.back().z)
	{
		//跳过重复点
		for (int i = path.size() - 2; i >= 0; -- i)
		{
			_path.push_back(path[i]);
		}
	}
	else
	{
		assert(false);
		return false;
	}
	return true;
}

bool PathTrack::Append(const PathTrack* path)
{
	if (!path)
	{
		return false;
	}
	return Append(path->GetPath());
}

bool PathTrack::SetCurPos(PointF& pt)
{
	if (_path.empty())
	{
		assert(false);
		return false;
	}

	if (_path.size() == 1)
	{
		_path[0] = pt;
		return true;
	}

	PointF dir(_path[1] - pt);
	float offsetX = fabs(dir.x);
	float offsetZ = fabs(dir.z);
	if (offsetX <= REPULSION_DISTANCE && offsetZ <= REPULSION_DISTANCE)
	{
		_path.erase(_path.begin());
	}
	else
	{
		_path[0] = pt;
	}

	return true;
}

float PathTrack::GetNextPos(PointF& next, float step)
{
	if (step <= 0.0f || _path.size() <= 0)
	{
		return -1.0f;
	}

	if (_path.size() == 1)
	{
		next = _path[0];
		return -1.0f;
	}

	float len_total = 0.0f;
	for (size_t i = 0; i < (_path.size()-1); ++ i)
	{
		PointF offset(_path[i+1] - _path[i]);
		float len = offset.Magnitude();
		if (step <= (len + len_total))
		{
			float rate = (step - len_total) / len;
			offset.x *= rate;
			offset.z *= rate;
			next = _path[i] + offset;
			if (i > 0)
			{
				_path.erase(_path.begin(), _path.begin() + i);
			}

			SetCurPos(next);
			return step;
		}
		else
		{
			len_total += len;
		}
	}

	if (_path.size() > 1)
	{
		_path.erase(_path.begin(), _path.begin() + _path.size() - 1);
	}

	next = _path[0];
	return len_total;
}

bool PathTrack::GoAcrossPath(std::vector<PointF>& path, float step)
{
	if (_path.empty())
	{
		return false;
	}

	path.clear();
	PointF next_pos;
	while (GetNextPos(next_pos, step) > 0.0f)
	{
		path.push_back(next_pos);
	};
	return true;
}

};
