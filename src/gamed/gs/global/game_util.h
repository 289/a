#ifndef GAMED_GS_GLOBAL_GAME_UTIL_H_
#define GAMED_GS_GLOBAL_GAME_UTIL_H_

#include <fstream>
#include "shared/base/base_define.h"
#include "shared/base/assertx.h"
#include "game_module/pathfinder/include/path_finder.h"

#include "math_types.h"


namespace gamed {

#define GS_INT32_MAX (std::numeric_limits<int32_t>::max())
#define GS_INT64_MAX (std::numeric_limits<int64_t>::max())

#define GS_INT32_MIN (std::numeric_limits<int32_t>::min())
#define GS_INT64_MIN (std::numeric_limits<int64_t>::min())

// type 移位成 mask
#ifndef SHIFT_TYPE
#define SHIFT_TYPE(type) (1 << type)
#endif // SHIFT_TYPE


///
/// utility function
///
inline int32_t second_to_tick(float sec)
{
	return static_cast<int32_t>(sec * TICK_PER_SEC + 0.5f);
}

inline int32_t millisecond_to_tick(float msec)
{
	return static_cast<int32_t>(msec * (TICK_PER_SEC/1000.0f) + 0.5f);
}

inline float millisecond_to_second(float msec)
{
	return msec / 1000.0f;
}

inline double second_to_millisecond(float sec)
{
	return (double)sec * 1000.0f;
}

inline int32_t index_to_mask(int32_t index)
{
	ASSERT(index >= 0);
	return 1 << index;
}


/*
 * @brief: 参数mask的二进制数里有且仅有一个1
 * @ret  : 返回二进制数里面第一个1在第几位
 */
inline int32_t mask_to_index(int32_t mask)
{
	ASSERT(mask && !(mask & (mask - 1)));
	int32_t n = 0;
	while(!(mask & (1 << n))) ++ n;
	return n;
}


///
/// path finder
///
inline A2DVECTOR PF_TO_A2D(const pathfinder::PF2DVECTOR& pf_vector)
{
	return A2DVECTOR(pf_vector.x, pf_vector.z);
}

inline pathfinder::PF2DVECTOR A2D_TO_PF(const A2DVECTOR& a2d_vector)
{
	return pathfinder::PF2DVECTOR(a2d_vector.x, a2d_vector.y);
}


///
/// linkid, sid - op function
///
inline int64_t pack_linkid_sid(int32_t linkid, int32_t sid)
{
	return makeInt64(linkid, sid);
}

inline void unpack_linkid_sid(int64_t param, int32_t& linkid, int32_t& sid)
{
	linkid = high32bits(param);
	sid    = low32bits(param);
}


///
/// time
///

/**
 * @brief time_is_overday 
 *    判断时间是否已经跨天
 */
// ---- thread safe ----
inline bool time_is_overday(time_t time, time_t now)
{
	struct tm tm_time;
	struct tm tm_now;
	localtime_r(&time, &tm_time);
	localtime_r(&now, &tm_now);
	if (tm_time.tm_year != tm_now.tm_year ||
		tm_time.tm_mon != tm_now.tm_mon ||
		tm_time.tm_mday != tm_now.tm_mday)
	{
		return true;
	}

	return false;
}


///
/// file
///

/**
 * @brief touchFile 
 *    判断文件是否存在
 */
inline bool touchFile(const char* file)
{
    bool res = false;
    std::ifstream ifs(file, std::ifstream::in);
    if (ifs) res = true;
    ifs.close();
    return res;
}

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GAME_UTIL_H_
