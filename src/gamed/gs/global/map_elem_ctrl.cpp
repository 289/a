#define __STDC_LIMIT_MACROS

#include "map_elem_ctrl.h"

#include "shared/logsys/logging.h"
#include "gs/template/extra_templ/extratempl_man.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/scene/world_cluster.h"

#include "game_util.h"
#include "timer.h"


namespace gamed {

MapElemCtrller::MapElemCtrller()
	: hb_ticker_(0),
	  check_countdown_(0)
{
}

MapElemCtrller::~MapElemCtrller()
{
	hb_ticker_       = 0;
	check_countdown_ = 0;
	active_ctrller_.clear();
	waiting_ctrller_.clear();
	invalid_ctrller_.clear();
	ctrller_info_vec_.clear();
}

bool MapElemCtrller::Init()
{
	std::vector<const extraTempl::MapElemCtrl*> tmp_vec;
	s_pExtraTempl->QueryExtraTemplByType<extraTempl::MapElemCtrl>(tmp_vec);

	for (size_t i = 0; i < tmp_vec.size(); ++i)
	{
        if (!tmp_vec[i]->time_seg.is_valid)
            continue;

		CtrllerInfo info;
		BuildTimeSlot(info.time_slot, tmp_vec[i]->time_seg);
		info.ptempl = tmp_vec[i];
		ctrller_info_vec_.push_back(info);
	}

	// after ctrller_info_vec_ init
	if (!BuildCtrllerList(g_timer->GetSysTime()))
	{
		LOG_ERROR << "MapElemCtrller - BuildCtrllerList failure!";
		return false;
	}
	size_t count = active_ctrller_.size() + waiting_ctrller_.size() + invalid_ctrller_.size();
	ASSERT(ctrller_info_vec_.size() == count);

	check_countdown_ = (tmp_vec.size() == 0) ? GS_INT32_MAX : 0;
	return true;
}

bool MapElemCtrller::BuildCtrllerList(time_t now)
{
	for (size_t i = 0; i < ctrller_info_vec_.size(); ++i)
	{
		const CtrllerInfo* info = &ctrller_info_vec_[i];
		ListInsertType type = InsertToCtrllerList(now, info);
		if (type == NOTHING_INSERTED)
		{
			return false;
		}
		else if (type == INSERT_TO_ACTIVE)
		{
			ActivateCtrller(*info);
		}
		else if (type == INSERT_TO_WAITING || type == INSERT_TO_INVALID)
		{
			// 可能地图元素是默认开启，这时要取消掉
			DeactivateCtrller(*info);
		}
	}
	return true;
}

MapElemCtrller::ListInsertType MapElemCtrller::InsertToCtrllerList(time_t now, const CtrllerInfo* info)
{
	time_t start = 0, end = 0;
	if (info->time_slot.GetNextTime(now, start, end))
	{
		if (now == start)
		{
			if (active_ctrller_.insert(std::make_pair(end, info)).second)
				return INSERT_TO_ACTIVE;
		}
		else
		{
			ASSERT(now < start);
			if (waiting_ctrller_.insert(std::make_pair(start, info)).second)
				return INSERT_TO_WAITING;
		}
	}
	else
	{
		if (invalid_ctrller_.insert(std::make_pair(0, info)).second)
			return INSERT_TO_INVALID;
	}

	return NOTHING_INSERTED;
}

void MapElemCtrller::HeartbeatTick()
{
	if (++hb_ticker_ >= TICK_PER_SEC)
	{
		if (--check_countdown_ <= 0)
		{
			check_countdown_ = CalcNextCheckoutTime(g_timer->GetSysTime());
		}
		hb_ticker_ = 0;
	}
}

int32_t MapElemCtrller::CalcNextCheckoutTime(time_t now)
{
	// active ctrller
	expired_vec_ = GetExpired(now, active_ctrller_);
	for (size_t i = 0; i < expired_vec_.size(); ++i)
	{
		const CtrllerInfo* info = expired_vec_[i].second;
		ListInsertType type = InsertToCtrllerList(now, info);
		if (type != INSERT_TO_ACTIVE)
		{
			DeactivateCtrller(*info);
		}
	}
	expired_vec_.clear();

	// waiting ctrller
	expired_vec_ = GetExpired(now, waiting_ctrller_);
	for	(size_t i = 0; i < expired_vec_.size(); ++i)
	{
		const CtrllerInfo* info = expired_vec_[i].second;
		ListInsertType type = InsertToCtrllerList(now, info);
		if (type == INSERT_TO_ACTIVE)
		{
			ActivateCtrller(*info);
		}
	}
	expired_vec_.clear();

	// check total count
	size_t count = active_ctrller_.size() + waiting_ctrller_.size() + invalid_ctrller_.size();
	ASSERT(ctrller_info_vec_.size() == count);

	int32_t next_start = 0, next_end = 0;
	next_end   = active_ctrller_.empty() ? GS_INT32_MAX : active_ctrller_.begin()->first;
	next_start = waiting_ctrller_.empty() ? GS_INT32_MAX : waiting_ctrller_.begin()->first;
	ASSERT(next_end >= now && next_start >= now);

	int32_t ret_secs = next_start - now;
	int32_t tmptime  = next_end - now;
	if (ret_secs > tmptime)
	{
		ret_secs = tmptime;
	}
	if (ret_secs > kMaxCheckoutTime)
	{
		ret_secs = kMaxCheckoutTime;
	}

	return ret_secs;
}

std::vector<MapElemCtrller::Entry> MapElemCtrller::GetExpired(time_t now, CtrllerList& ctrller_list)
{
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<const CtrllerInfo*>(UINTPTR_MAX));
	CtrllerList::iterator end = ctrller_list.lower_bound(sentry);
	ASSERT(end == ctrller_list.end() || now < end->first);
	std::copy(ctrller_list.begin(), end, back_inserter(expired));
	ctrller_list.erase(ctrller_list.begin(), end);
	return expired;
}

void MapElemCtrller::ActivateCtrller(const CtrllerInfo& info)
{
	for (size_t i = 0; i < info.ptempl->map_elem_list.size(); ++i)
	{
		EnableMapElem(info.ptempl->map_elem_list[i]);
	}
}

void MapElemCtrller::DeactivateCtrller(const CtrllerInfo& info)
{
	for (size_t i = 0; i < info.ptempl->map_elem_list.size(); ++i)
	{
		DisableMapElem(info.ptempl->map_elem_list[i]);
	}
}

void MapElemCtrller::EnableMapElem(int32_t elemid)
{
	const mapDataSvr::BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elemid);
	if (pdata == NULL)
		return;

	wcluster::EnableMapElem(pdata->map_id, elemid);
}

void MapElemCtrller::DisableMapElem(int32_t elemid)
{
	const mapDataSvr::BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(elemid);
	if (pdata == NULL)
		return;

	wcluster::DisableMapElem(pdata->map_id, elemid);
}

} // namespace gamed

#undef __STDC_LIMIT_MACROS
