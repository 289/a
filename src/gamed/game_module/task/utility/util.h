#ifndef TASK_UTIL_H_
#define TASK_UTIL_H_

#include <stdint.h>
#include <vector>

namespace task
{

class ZoneInfo;
class Position;

// 通用辅助函数
class Util
{
public:
	static int32_t Rand(int32_t lower, int32_t upper);
	static int32_t Rand(const std::vector<int32_t>& prob_vec);
	static int32_t Rand();

    static bool IsNormalMap(int32_t id);
    static bool IsDramaMap(int32_t id);
    static bool IsBGMap(int32_t id);
    static bool IsInsMap(int32_t id);
    static bool IsEqual(const Position& pos, int32_t world_id, float x, float y);
    static bool IsInZone(const ZoneInfo& info, int32_t world_id, float x, float y);
    static bool MatchZoneOp(const ZoneInfo& info, int32_t world_id, float x, float y, int8_t op);
private:
    static int32_t WorldIdToMapId(int32_t id);
private:
    static const int32_t kMapIdOffset = 1000;
    static const int32_t kInsMapIdPrefix = 6;
    static const int32_t kBGMapIdPrefix = 7;
    static const int32_t kDramaMapIdPrefix = 2;
};

} // namespace task

#endif // TASK_UTIL_H_
