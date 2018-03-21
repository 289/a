#include <assert.h>
#include "util.h"
#include "shared/security/randomgen.h"
#include "basic_info.h"
#include "zone_info.h"

namespace task
{

using namespace std;

// 随机数相关

int32_t Util::Rand(int32_t lower, int32_t upper)
{
	assert(upper >= lower);
	return shared::net::RandomGen::RandUniform(lower, upper);
}

int32_t Util::Rand()
{
	return Rand(1, 10000);
}

int32_t Util::Rand(const vector<int32_t>& prob_vec)
{
	int32_t prob_size = prob_vec.size();
    if (prob_size == 0)
    {
        assert(false);
        return -1;
    }

	int32_t prob = 0, index = 0;
	for (index = 0; index < prob_size; ++index)
	{
		prob += prob_vec[index];
	}
    if (prob == 0)
    {
        return 0;
    }

	int32_t rand_num = Rand(1, prob);
	prob = 0;
	for (index = 0; index < prob_size; ++index)
	{
		prob += prob_vec[index];
		if (rand_num <= prob)
		{
			return index;
		}
	}
	assert(false);
	return 0;
}

int32_t Util::WorldIdToMapId(int32_t id)
{
	return id & 0xFFFF;
}

bool Util::IsInsMap(int32_t id)
{
	id = WorldIdToMapId(id);
	if ((id / kMapIdOffset) == kInsMapIdPrefix)
	{
		return true;
	}
	return false;
}

bool Util::IsDramaMap(int32_t id)
{
	id = WorldIdToMapId(id);
    if ((id / kMapIdOffset) == kDramaMapIdPrefix)
        return true;
	return false;
}

bool Util::IsBGMap(int32_t id)
{
	id = WorldIdToMapId(id);
	if ((id / kMapIdOffset) == kBGMapIdPrefix)
	{
		return true;
	}
	return false;
}

bool Util::IsNormalMap(int32_t id)
{
	if (IsInsMap(id))
		return false;
    if (IsBGMap(id))
        return false;
	return true;
}

bool Util::IsEqual(const Position& pos, int32_t world_id, float x, float y)
{
	int32_t x_diff = (int32_t)(pos.x - x);
	int32_t y_diff = (int32_t)(pos.y - y);
	return pos.world_id == world_id && x_diff == 0 && y_diff == 0;
}

bool Util::IsInZone(const ZoneInfo& info, int32_t world_id, float x, float y)
{
    if (info.world_id == 0)
    {
        return Util::IsNormalMap(world_id);
    }

	if (info.world_id != world_id)
	{
		return false;
	}
	return x >= info.min_x && x <= info.max_x && y >= info.min_y && y <= info.max_y;
}

bool Util::MatchZoneOp(const ZoneInfo& info, int32_t world_id, float x, float y, int8_t op)
{
    bool in_zone = IsInZone(info, world_id, x, y);
	return (op == ZONE_ENTER && in_zone) || (op == ZONE_LEAVE && !in_zone);
}

} // namespace task
