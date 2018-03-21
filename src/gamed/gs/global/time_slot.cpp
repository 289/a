#include "time_slot.h"

#include "shared/base/assertx.h"

#include "gs/template/data_templ/base_datatempl.h"
#include "gs/template/extra_templ/base_extratempl.h"


namespace gamed {

TimeSlot::TimeSlot()
	: has_added_(false)
{
}

TimeSlot::~TimeSlot()
{
	ClearEntity();
}

void TimeSlot::AddEntity(const Entity& entity)
{
	ASSERT(!has_added_);

	time_seg_.set_start_date(entity.start_date);
	time_seg_.set_end_date(entity.end_date);

	shared::TimeEntry time_ent;
	time_ent.set_start_time(entity.start_time);
	time_ent.set_end_time(entity.end_time);
	time_ent.set_day_of_month(entity.is_day_of_month);
	for (size_t i = 0; i < entity.days.size(); ++i)
	{
		time_ent.add_day(entity.days[i]);
	}
	for (size_t i = 0; i < entity.months.size(); ++i)
	{
		time_ent.add_month(entity.months[i]);
	}

	time_seg_.add_entry(time_ent);
	has_added_ = true;

	// end
	time_seg_.Finalize();
}

void TimeSlot::ClearEntity()
{
	time_seg_.set_start_date(0);
	time_seg_.set_end_date(0);
	time_seg_.clear_entry();
	has_added_ = false;
}

bool TimeSlot::IsMatch(time_t now) const
{
	ASSERT(has_added_);
	return time_seg_.IsMatch(now);
}

bool TimeSlot::GetNextTime(time_t now, time_t& start, time_t& end) const
{
	ASSERT(has_added_);
	return time_seg_.GetNextTime(now, start, end);
}

void BuildTimeSlot(TimeSlot& tslot, const dataTempl::TimeSegment& tseg)
{
	TimeSlot::Entity entity;
	entity.start_date = tseg.start_date;
	entity.end_date = tseg.end_date;
	entity.start_time = tseg.start_time;
	entity.end_time = tseg.end_time;
	entity.is_day_of_month = tseg.is_day_of_month;

	entity.days.resize(tseg.days.size());
	for (size_t i = 0; i < tseg.days.size(); ++ i)
		entity.days[i] = tseg.days[i];

	entity.months.resize(tseg.months.size());
	for (size_t i = 0; i < tseg.months.size(); ++ i)
		entity.months[i] = tseg.months[i];

	tslot.AddEntity(entity);
}

void BuildTimeSlot(TimeSlot& tslot, const extraTempl::TimeSegment& tseg)
{
	TimeSlot::Entity entity;
	entity.start_date = tseg.start_date;
	entity.end_date = tseg.end_date;
	entity.start_time = tseg.start_time;
	entity.end_time = tseg.end_time;
	entity.is_day_of_month = tseg.is_day_of_month;

	entity.days.resize(tseg.days.size());
	for (size_t i = 0; i < tseg.days.size(); ++ i)
		entity.days[i] = tseg.days[i];

	entity.months.resize(tseg.months.size());
	for (size_t i = 0; i < tseg.months.size(); ++ i)
		entity.months[i] = tseg.months[i];

	tslot.AddEntity(entity);
}

} // namespace gamed
