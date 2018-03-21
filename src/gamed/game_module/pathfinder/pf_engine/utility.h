#ifndef __PF_UTILITY_H__
#define __PF_UTILITY_H__

#include "shared/net/buffer.h"

namespace pathfinder
{

/**
 * @func  Distance
 * @brief 计算原始路径的长度
 */
inline float Distance(const Path& path)
{
	if (path.size() <= 1)
	{
		return 0.0f;
	}

	float distance = 0.0f;
	for (size_t i = 0; i < path.size()-1; ++ i)
	{
		PointI offset = path[i+1] - path[i];
		if (abs(offset.x) + abs(offset.z) == 1)
		{
			distance += 1.0f;
		}
		else if (abs(offset.x) + abs(offset.z) == 2)
		{
			distance += 1.414f;
		}
		else
		{
			assert(false);
			return -1.0f;
		}
	}

	return distance;
}

inline float ReadFloatFromBuffer(shared::net::Buffer& buffer)
{
	float result = 0.0f;
	if (buffer.ReadableBytes() < sizeof(float))
		return -1.0f;

	::memcpy(&result, buffer.peek(), sizeof(float));
	buffer.Retrieve(sizeof(float));
	return result;
}

};

#endif // __PF_UTILITY_H__
