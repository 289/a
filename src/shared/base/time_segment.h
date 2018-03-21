#ifndef SHARED_BASE_TIME_SEGMENT_H_
#define SHARED_BASE_TIME_SEGMENT_H_

#include <vector>
#include "time_entry.h"


namespace shared {

//
// 时间段
// 功能：
//		与时间条目相同，但由多个时间条目共同限定，每个条目之间是"或"的关系
//		另外还可以设置时间段条件作用的起始和结束时间
// 成员变量：
//		start_date_/end_date_：时间段起始，结束时间，如果为0表示不限制（任意年）
//		entries_：时间条目集合
//
class TimeSegment : public shared::copyable
{
public:
	TimeSegment();
	void Finalize();
	bool IsMatch(time_t time) const;
	bool GetNextTime(time_t time, time_t& start, time_t& end) const;

	void UTCToLocal();

	inline void set_start_date(time_t start);
	inline void set_end_date(time_t end);
	inline void add_entry(const TimeEntry& entry);
	inline void clear_entry();

public:
	int32_t start_date_;
	int32_t end_date_;

	std::vector<TimeEntry> entries_;
};

inline void TimeSegment::set_start_date(time_t start)
{
	start_date_ = start;
}

inline void TimeSegment::set_end_date(time_t end)
{
	end_date_ = end;
}

inline void TimeSegment::add_entry(const TimeEntry& entry)
{
	entries_.push_back(entry);
}

inline void TimeSegment::clear_entry()
{
	entries_.clear();
}

} // namespace shared

#endif // SHARED_BASE_TIME_SEGMENT_H_
