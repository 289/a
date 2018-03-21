#ifndef GAMED_GS_GLOBAL_TIME_SEGMENT_H_
#define GAMED_GS_GLOBAL_TIME_SEGMENT_H_

#include "shared/base/copyable.h"
#include "shared/base/time_segment.h"


namespace dataTempl {
	struct TimeSegment;
};

namespace extraTempl {
	struct TimeSegment;
};


namespace gamed {

/**
 * @brief TimeSlot
 */
class TimeSlot : public shared::copyable
{
public:
	/**
	 * @brief 时间段
	 *    (1)start_date，end_date必须同时有值或无值
	 *    (2)start_time，end_time必须同时有值或无值
	 *
	 * 成员变量：
	 *      months：哪些月份满足要求
	 *      is_day_of_month：指定days表示几号还是星期几
	 *      days：哪些日子满足要求
	 *      start_time/end_time：当日的时分秒处于该范围内才满足要求
	 * 取值范围限制：
	 *      months：size小于等于12，值的有效范围[0,11]
	 *      days：当day_of_month_为true时，size小于等于31，值的有效范围[1,31]
	 *            当day_of_month_为false时，size小于等于7，值的有效范围[0,6]
	 *      start_time/end_time：前者小于等于后者，有效范围[0,23*3600+59*60+59]
	 */
	struct Entity
	{
		int32_t start_date;  // 绝对时间，调整过的开启时间，默认值0表示不限制
		int32_t end_date;    // 绝对时间，调整过的结束时间，默认值同上
		int32_t start_time;  // 当天开启时间，记录的是秒数，默认值同上
		int32_t end_time;    // 当天结束时间，记录的是秒数，默认值同上
		int8_t  is_day_of_month;    // days是不是日，默认值true表示days里保存的是日，false表示保存的是星期几
		std::vector<int8_t> days;   // 保存日或者星期几，空表示不限制
		std::vector<int8_t> months; // 保存月份，空表示不限制
	};

public:
	TimeSlot();
	~TimeSlot();

	// 所给时间是否在时间段内
	bool IsMatch(time_t now) const;

	// 返回false表示时间段已经过期
	bool GetNextTime(time_t now, time_t& start, time_t& end) const;

	// 每次只能add一个Entity，重复使用需要先调Clear清除原来的Entity
	void AddEntity(const Entity& ent);
	void ClearEntity();

private:
	bool has_added_;
	shared::TimeSegment time_seg_;
};

///
/// 功能函数
///

// 通过dataTempl::TimeSegment初始化构造TimeSlot
void BuildTimeSlot(TimeSlot& tslot, const dataTempl::TimeSegment& tseg);

// 通过extraTempl::TimeSegment初始化构造TimeSlot
void BuildTimeSlot(TimeSlot& tslot, const extraTempl::TimeSegment& tseg);

} // namespace gamed

#endif // GAMED_GS_GLOBAL_TIME_SEGMENT_H_
