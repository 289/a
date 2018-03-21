#ifndef SHARED_BASE_TIME_ENTRY_H_
#define SHARED_BASE_TIME_ENTRY_H_

#include <stdint.h>
#include <time.h>
#include <set>

#include "shared/base/copyable.h"


namespace shared {

//
// 时间条目
// 功能：
//		判断某时间是否满足时间条目的要求，以及获取满足要求的时间
// 成员变量：
//		months_：哪些月份满足要求
//		day_of_month_：指定days_表示几号还是星期几
//		days_：哪些日子满足要求
//		start_time_/end_time_：当日的时分秒处于该范围内才满足要求
// 取值范围限制：
//		months_：size的有效范围[1,12]，值的有效范围[0,11]
//		days_：当day_of_month_为true时，size的有效范围[1,31]，值的有效范围[1,31]
//			   当day_of_month_为false时，size的有效范围[1,7]，值的有效范围[0,6]
//		start_time_/end_time_：前者小于等于后者，有效范围[0,23*3600+59*60+59]
//
class TimeEntry : public shared::copyable
{
public:
	TimeEntry();
	void Finalize();
	bool IsMatch(time_t time) const;
	// time对应的年内存在满足要求的时间，则start，end返回距离time最近的满足要求的时间段
	// time当年内没有满足要求的时间，则start=end=下一年1月1日0:0:0对应的秒数
	// 返回的start，end属于同一天
	void GetNextTime(time_t time, time_t& start, time_t& end) const;

	inline void set_day_of_month(bool value);
	inline void add_day(int8_t day);
	inline void clear_days();
	inline void add_month(int8_t month);
	inline void clear_months();
	inline void set_start_time(int32_t start);
	inline void set_end_time(int32_t end);

private:
	bool IsDateMatch(const tm& date) const;
	bool OnGetNextTime(const tm& time, tm& next) const;
	bool CalcNextByMday(tm& next) const;
	bool CalcNextByWday(tm& next) const;

public:
	std::set<int8_t> months_;
	bool day_of_month_;
	std::set<int8_t> days_;

	int32_t start_time_;
	int32_t end_time_;
};

inline void TimeEntry::set_day_of_month(bool value)
{
	day_of_month_ = value;
}

inline void TimeEntry::add_day(int8_t day)
{
	days_.insert(day);
}

inline void TimeEntry::clear_days()
{
	days_.clear();
}

inline void TimeEntry::add_month(int8_t month)
{
	months_.insert(month);
}

inline void TimeEntry::clear_months()
{
	months_.clear();
}

inline void TimeEntry::set_start_time(int32_t start)
{
	start_time_ = start;
}

inline void TimeEntry::set_end_time(int32_t end)
{
	end_time_ = end;
}

} // namespace shared

#endif // SHARED_BASE_TIME_ENTRY_H_
