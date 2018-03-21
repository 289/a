#include "base_extratempl.h"

#ifdef PLATFORM_WINDOWS
#include <time.h>
#endif // PLATFORM_WINDOWS

// templ header
#include "monster_group.h"
#include "battle_scene.h"
#include "mapelem_ctrl.h"
#include "client_map_global.h"


namespace extraTempl {

// init template
INIT_STAITC_EXTRATEMPLATE(MonsterGroupTempl, TEMPL_TYPE_MONSTER_GROUP_TEMPL);
INIT_STAITC_EXTRATEMPLATE(BattleSceneTempl, TEMPL_TYPE_BATTLE_SCENE_TEMPL);
INIT_STAITC_EXTRATEMPLATE(MapElemCtrl, TEMPL_TYPE_MAPELEM_CTRL_TEMPL);
INIT_STAITC_EXTRATEMPLATE(ClientMapGlobal, TEMPL_TYPE_CLIENT_MAP_GLOBAL);


///
/// TimeSegment
///
TimeSegment::TimeSegment()
	: is_valid(0),
	  start_date(0),
	  end_date(0),
	  start_time(0),
	  end_time(0),
	  is_day_of_month(1)
{
}

void TimeSegment::Pack(shared::net::ByteBuffer& buf)
{
    PACK_NESTED_VALUE(is_valid, start_date, end_date, start_time, end_time, is_day_of_month, days, months);
}

void TimeSegment::UnPack(shared::net::ByteBuffer& buf)
{
    UNPACK_NESTED_VALUE(is_valid, start_date, end_date, start_time, end_time, is_day_of_month, days, months);

    if (is_valid) 
    {
        CorrectionDateTime(*this);
        assert(CheckTimeSegment(*this));
    }
}

bool CheckTimeSegment(const TimeSegment& time_seg)
{
	const int sec_limit = 23*3600 + 59*60 + 59;

	//
	// start_date/end_date
	//
	if (time_seg.start_date <= 0 && time_seg.end_date > 0)
		return false;
	if (time_seg.start_date > 0 && time_seg.end_date <= 0)
		return false;
	if (time_seg.start_date > 0 && time_seg.start_date > time_seg.end_date)
		return false;

	//
	// start_time/end_time
	//
	if (time_seg.start_time < 0 && time_seg.end_time > 0)
		return false;
	if (time_seg.start_time > 0 && time_seg.end_time < 0)
		return false;
	if (time_seg.start_time < 0 || time_seg.start_time > sec_limit || 
		time_seg.end_time < 0 || time_seg.end_time > sec_limit)
		return false;
	if (time_seg.start_time > 0 && time_seg.start_time > time_seg.end_time)
		return false;

	//
	// days
	//
	if (!time_seg.is_day_of_month && time_seg.days.size() > 7)
		return false;

	for (size_t i = 0; i < time_seg.days.size(); ++i)
	{
		if (time_seg.is_day_of_month)
		{
			if (time_seg.days[i] < 1 || time_seg.days[i] > 31)
				return false;
		}
		else
		{
			if (time_seg.days[i] < 0 || time_seg.days[i] > 6)
				return false;
		}
	}

	//
	// months
	//
	for (size_t i = 0; i < time_seg.months.size(); ++i)
	{
		if (time_seg.months[i] < 0 || time_seg.months[i] > 11)
			return false;
	}

	return true;
}

void CorrectionDateTime(TimeSegment& time_seg)
{
	//
	// 时间调整
	//
	if (time_seg.start_date > 0 && time_seg.end_date > 0)
	{
		time_t seconds = time(NULL);
		struct tm tm_time;
#ifdef PLATFORM_WINDOWS
		localtime_s(&tm_time, (const time_t*)&seconds);
		struct tm tm_gmt;
		gmtime_s(&tm_gmt, (const time_t*)&seconds);
		int tz_zone = tm_time.tm_hour - tm_gmt.tm_hour;
		int tz_adjust = -(tz_zone * 3600);
#else // !PLATFORM_WINDOWS
		localtime_r(&seconds, &tm_time);
		int tz_adjust = -tm_time.tm_gmtoff;
#endif // PLATFORM_WINDOWS
		time_seg.start_date += tz_adjust;
		time_seg.end_date   += tz_adjust;
	}
}


///
/// class BaseExtraTempl
///
void BaseExtraTempl::Marshal()
{
	MARSHAL_TEMPLVALUE(templ_id);
	OnMarshal();
}

void BaseExtraTempl::Unmarshal()
{
	UNMARSHAL_TEMPLVALUE(templ_id);
	OnUnmarshal();
}

bool BaseExtraTempl::CheckDataValidity()
{
	return OnCheckDataValidity();
}


///
/// class BaseExtraTemplManager
///
BaseExtraTempl* BaseExtraTemplManager::CreatePacket(BaseExtraTempl::Type id)
{
	return BaseExtraTemplManager::GetInstance()->OnCreatePacket(id);;
}

bool BaseExtraTemplManager::InsertPacket(uint16_t type, BaseExtraTempl* packet)
{
	return BaseExtraTemplManager::GetInstance()->OnInsertPacket(type, packet);
}

bool BaseExtraTemplManager::IsValidType(int32_t type)
{
	if (type > TEMPL_TYPE_INVALID && type < TEMPL_TYPE_MAX)
	{
		return true;
	}

	return false;
}

BaseExtraTempl* BaseExtraTemplManager::OnCreatePacket(BaseExtraTempl::Type id)
{
	BaseExtraTemplMap::iterator it = packet_map_.find(id);
	if (packet_map_.end() == it) return NULL;

	return dynamic_cast<BaseExtraTempl*>(it->second->Clone());
}
	
bool BaseExtraTemplManager::OnInsertPacket(uint16_t type, BaseExtraTempl* packet)
{
	if (!packet_map_.insert(std::make_pair(type, packet)).second)
	{
		assert(false);
		return false;
	}

	return true;
}

} // namespace extraTempl
