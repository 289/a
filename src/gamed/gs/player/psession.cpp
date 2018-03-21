#include "psession.h"

#include "gs/global/message.h"
#include "gs/global/msg_obj.h"
#include "gs/global/game_util.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"

#include "player_sender.h"
#include "player.h"
#include "gs/player/filter/invincible_filter.h"
#include "gs/global/game_util.h"


namespace gamed {

///
/// PlayerSession
///
PlayerSession::PlayerSession(Player* player)
	: ActiveSession(player),
	  pplayer_(player)
{
}

PlayerSession::~PlayerSession()
{
}

bool PlayerSession::TerminateSession() 
{ 
	EndSession(); 
	return true; 
}

///
/// PSayHelloSession
///
bool PSayHelloSession::StartSession(const ActiveSession* next_ses)
{
	pplayer_->SayHelloToNpc(target_);
	return false; 
}

///
/// PRegionTransportSession
///
void PRegionTransportSession::SetTransportTarget(int32_t world_id, const A2DVECTOR& pos)
{
	target_world_id_ = world_id;
	target_pos_      = pos;
}

bool PRegionTransportSession::StartSession(const ActiveSession* next_ses)
{
	if (!pplayer_->RegionTransport(target_world_id_, target_pos_))
	{
		pplayer_->sender()->ErrorMessage(G2C::ERR_CANNOT_REGION_TRANSPORT);
	}
	return false;
}


///
/// PGatherPrepareSession
///
void PGatherPrepareSession::SetTarget(const XID& matterxid, int32_t gather_seq_no, int32_t gather_timeout)
{
	matter_xid_     = matterxid;
	gather_seq_no_  = gather_seq_no;
	gather_timeout_ = gather_timeout;
}

bool PGatherPrepareSession::StartSession(const ActiveSession* next_ses) 
{ 
	msg_gather_request param;
	param.gather_seq_no  = gather_seq_no_;
	param.gather_timeout = gather_timeout_;
	pplayer_->SendMsg(GS_MSG_GATHER_REQUEST, matter_xid_, &param, sizeof(param));
	return false;  // 一定要return false
}


///
/// PGatherSession
///
void PGatherSession::SetTarget(const XID& matterxid, int32_t gather_time, int32_t gather_seq_no, int32_t mattertid)
{
	matter_xid_      = matterxid;
	matter_tid_      = mattertid;
	gather_time_     = gather_time;
	gather_seq_no_   = gather_seq_no;
}

void PGatherSession::SetTriggerTask(int32_t task_id, int32_t task_prob, int32_t task_interval, int32_t task_times)
{
    tg_task_id_       = task_id;
    tg_task_prob_     = task_prob;
    tg_task_interval_ = task_interval;
    tg_task_times_    = task_times;
}

void PGatherSession::DeliverTask()
{
    if (--tg_task_times_ < 0)
        return;

    if (!mrand::RandSelect(tg_task_prob_))
        return;

    // 发任务
    if (pplayer_->DeliverTask(tg_task_id_) == 0)
    {
        __PRINTF("玩家%ld采矿过程中接到任务........task_id:%d", pplayer_->role_id(), tg_task_id_);
    }
}

bool PGatherSession::StartSession(const ActiveSession* next_ses)
{
	gather_flag_ = true;

	// 开始采集
	pplayer_->sender()->GatherStart(matter_xid_.id, gather_time_, gather_seq_no_, matter_tid_);

    // 计算不同的时间方式
    if (tg_task_id_ > 0 && tg_task_interval_ < gather_time_)
    {
        int tick      = millisecond_to_tick(tg_task_interval_);
        int times     = gather_time_ / tg_task_interval_ + 1; // 多加1
        AutoSetTimer(tick, times);
    }
    else
    {
        int tick = millisecond_to_tick(gather_time_);
        AutoSetTimer(tick, 1);
    }
	return true;
}

bool PGatherSession::RepeatSession(int times)
{
    // 发任务
    DeliverTask();    

    // 最后一次repeat，需要重新计算时间
    if (times <= 0)
    {
        int realTime = gather_time_ % tg_task_interval_;
        if (realTime <= 0)
            return false;

        int tick = millisecond_to_tick(realTime);
        AutoSetTimer(tick, 1);
        return true;
    }
	return true;
}

bool PGatherSession::Mutable(const ActiveSession* next_ses)
{
	if (next_ses == NULL) 
		return false;

	stop_reason_    = 2;
	bool is_success = false;
	if (next_ses->GetMask() == ActiveSession::AS_MASK_GATHER_RESULT)
	{
		const PGatherResultSession* psession = dynamic_cast<const PGatherResultSession*>(next_ses);
		if (psession)
		{
			if (psession->IsGatherSuccess(matter_xid_, gather_seq_no_))
			{
				is_success   = true;
				stop_reason_ = 0;
			}
			else 
			{
				is_success   = false;
				stop_reason_ = psession->GetFailureReason();
			}
		}
	}

	if (is_success)
	{
		gather_flag_ = true;
	}
	else
	{
		gather_flag_ = false;
	}
	return true;
}

bool PGatherSession::EndSession()
{
    if (tg_task_id_ > 0)
    {
        // 最后一次任务发放
        DeliverTask();
    }

	ASSERT(session_id() != -1);
	if (gather_flag_)
	{
		// 正常采集完毕，发送采集消息
		pplayer_->SendMsg(GS_MSG_GATHER_COMPLETE, matter_xid_, gather_seq_no_);
		ASSERT(stop_reason_ == 0);
	}
	else
	{
		// 采集中断，发送中断采集消息
		pplayer_->SendMsg(GS_MSG_GATHER_CANCEL, matter_xid_, gather_seq_no_);
		if (!stop_reason_) stop_reason_ = 3;
	}

	// shit!
	if (stop_reason_ == 0 && gather_time_ == 0)
	{
		stop_reason_ = 4;
	}

	pplayer_->sender()->GatherStop(matter_xid_.id, stop_reason_, matter_tid_);
	return true;
}

bool PGatherSession::TerminateSession()
{
	gather_flag_ = false;
	EndSession();
	return true;
}


///
/// PGatherResultSession
///
void PGatherResultSession::SetTarget(const XID& matterxid, int32_t gather_seq_no, bool is_success, int8_t reason)
{
	matter_xid_    = matterxid;
	gather_seq_no_ = gather_seq_no;
	is_success_    = is_success;
	reason_        = reason;
}

bool PGatherResultSession::GetFailureReason() const
{
	return reason_;
}

bool PGatherResultSession::IsGatherSuccess(const XID& matterxid, int32_t gather_seq_no) const
{
	if (gather_seq_no != gather_seq_no_ || matterxid != matter_xid_)
	{
		return false;
	}
	return is_success_;
}


///
/// PInvincibleSession
///
bool PInvincibleSession::StartSession(const ActiveSession* next_ses)
{
	int32_t tick = millisecond_to_tick(time_*1000);
	AutoSetTimer(tick, 1);
	pplayer_->AddFilter(new InvincibleFilter(pplayer_, 0, time_));
	return true;
}

bool PInvincibleSession::EndSession()
{
    pplayer_->DelFilter(FILTER_TYPE_INVINCIBLE);
	return true;
}

} // namespace gamed
