#include "pmv_session.h"

#include "gs/global/timer.h"
#include "gs/global/game_util.h"
#include "gs/global/game_def.h"
#include "gs/global/dbgprt.h"
#include "gs/player/player.h"
#include "gs/player/player_sender.h"

#include "player_move_ctrl.h"
#include "move_util.h"


namespace gamed {

///
/// PStartMoveSession
///
PStartMoveSession::PStartMoveSession(Player* player)
	: PlayerSession(player),
	  mv_ctrl_(*(player->move_ctrl())),
	  base_time_(0)
{
}

PStartMoveSession::~PStartMoveSession()
{
}

void PStartMoveSession::SetStartMoveInfo(A2DVECTOR& dest, uint16_t speed, uint8_t mode)
{
	dest_     = dest;
	speed_    = speed;  // 百分数
	mode_     = mode;
}

bool PStartMoveSession::StartSession(const ActiveSession* next_ses)
{
	pplayer_->sender()->MoveStart(dest_, speed_, mode_);
	MOVE_PRINTF("MMMM START MMM: (%6.2f, %6.2f) ", dest_.x, dest_.y);
	mv_ctrl_.SetSessionStartInfo(mv_ctrl_.GetCurMoveSeq(), session_id());

	// calculate ticks than will hold in session
	mv_ctrl_.set_start_time(g_timer->GetSysTimeMsecs()); 
	int tick = millisecond_to_tick(kStdMoveUseTime);
	if (next_ses)
	{
		// 如果后面的movecontinue,或者movestop已经到达（可能服务器很卡）
		// 则直接使用这个时间来做延迟
		int next_time = next_ses->GetMoveTime();
		if (next_time > 0)
		{
			// 要注意防止客户端的欺骗
			tick = millisecond_to_tick(next_time);
			if (tick <= 0) tick = 1;
		}
	}
	AutoSetTimer(tick, 1);
	return true;
}

bool PStartMoveSession::Mutable(const ActiveSession* next_ses)
{
	if (next_ses == NULL || mv_ctrl_.ses_start_seq() < 0) return false;

	// move后续的协议提前到达，需要重新计算时间延迟，以及防止作弊
	// next_time就是被调整过的cli_use_time
	int next_time = next_ses->GetMoveTime();
	if (next_time > 0)
	{
		int64_t end = g_timer->GetSysTimeMsecs();
		int svr_use_time = end - mv_ctrl_.start_time() + base_time_;
		int delta_t = next_time - svr_use_time;
		if (delta_t < kFastMoveSkipTime)
		{
			// 如果时间差在容许范围之内，则由下一个move session来处理时间延迟
			return true;
		}

		// 重新设置timer
		AutoSetTimer(millisecond_to_tick(delta_t), 1);
		return false;
	}

	return true;
}

bool PStartMoveSession::EndSession()
{
	ASSERT(session_id() != -1)

	// session id 可能会在Mutable里改变，所以需要重新设置
	mv_ctrl_.SetSessionStartInfo(mv_ctrl_.GetCurMoveSeq(), session_id());
#ifdef __TEST_MOVE_DELAY__
	int64_t end = g_timer.GetSysTimeMsecs();
	__PRINTF("startmove session use %lldmsecs", end - mv_ctrl_.start_time());
#endif
	return true;
}

int PStartMoveSession::GetMask() const
{
	return AS_MASK_MOVE;
}

int PStartMoveSession::GetExclusiveMask()
{
	return ~(AS_MASK_MOVE);
}

///
/// PMoveSession
///
PMoveSession::PMoveSession(Player* player)
	: PStartMoveSession(player),
	  cli_use_time_(0)
{
}

PMoveSession::~PMoveSession()
{
}

void PMoveSession::SetMoveInfo(int32_t seq, uint16_t speed, uint8_t mode,
							   A2DVECTOR& cur_pos, A2DVECTOR& dest, int32_t use_time)
{
	seq_from_client_ = seq;
	speed_           = speed;  // 百分数
	mode_            = mode;
	cli_cur_pos_     = cur_pos;
	dest_            = dest;
	cli_use_time_    = use_time;
}

bool PMoveSession::StartSession(const ActiveSession* next_ses)
{
	A2DVECTOR old_pos = pplayer_->pos();
	if (mv_ctrl_.ExecuteMovement(seq_from_client_, old_pos, cli_cur_pos_, cli_use_time_))
	{
		// 注意speed是一个百分数
		pplayer_->sender()->Move(cli_cur_pos_, dest_, speed_, mode_);
		MOVE_PRINTF("MMMM MOVE MMM: (%6.2f, %6.2f) ----------> (%6.2f, %6.2f) ", old_pos.x, old_pos.y, 
                pplayer_->pos().x, pplayer_->pos().y);

		int tick = 0;
		if (!mv_ctrl_.CheckSeqValid(session_id()))
		{
			// 不是正常的move，使用通常的方式
			mv_ctrl_.SetSessionStartInfo(-1, -1);
			tick = millisecond_to_tick(cli_use_time_);
			if (tick < 8) tick = 8;
		}
		else
		{
			tick = CalculateTimeDelay(next_ses);
			mv_ctrl_.SetSessionStartInfo(mv_ctrl_.GetCurMoveSeq(), session_id());
		}

		AutoSetTimer(tick, 1);	
		return true;
	}
	return false;
}

int PMoveSession::CalculateTimeDelay(const ActiveSession* next_ses)
{
	// 计算延迟
	int64_t end = g_timer->GetSysTimeMsecs();
	int svr_use_time = end - mv_ctrl_.start_time();
	int delta_t = svr_use_time - cli_use_time_;
	if (delta_t > 0)
	{
		// 服务器用时多
		if (delta_t > kMinMoveUseTime) delta_t = kMinMoveUseTime;
		base_time_ = delta_t;
	}
	else
	{
		// 服务器用时少
		//
		if (delta_t < -kMaxMoveUseTime) delta_t = -kMaxMoveUseTime;
		base_time_ = delta_t;
	}

	mv_ctrl_.set_start_time(end); // 从现在开始计时，计算结果已经记入base_time_

	// 按照base_time_来设定延迟时间
	int tick = -base_time_;

	// 如果后续session已到达
	int next_tick = kStdMoveUseTime;
	if (next_ses)
	{
		int next_time = next_ses->GetMoveTime();
		if (next_time > 0) next_tick = next_time;  // 此项操作是为了避免客户端欺骗
	}

	tick += next_tick;
	tick = millisecond_to_tick(tick);
	if (tick <= 0) tick = 1;

	return tick;
}

bool PMoveSession::EndSession()
{
	ASSERT(session_id() != -1)

	// session id 可能会在Mutable里改变，所以需要重新设置
	mv_ctrl_.SetSessionStartInfo(mv_ctrl_.GetCurMoveSeq(), session_id());

	// 这里重新设置start_time其实是一种时延的补偿
	int64_t time = mv_ctrl_.start_time() - base_time_;
	mv_ctrl_.set_start_time(time);
	return true;
}

int PMoveSession::GetMoveTime() const
{
	int t = cli_use_time_;
	if (t <= 0) {
		t = kMinMoveUseTime;
	}
	if (t > kMaxMoveUseTime) {
		t = kMaxMoveUseTime;
	}
	return t;
}

///
/// PStopMoveSession
///
PStopMoveSession::PStopMoveSession(Player* player)
	: PMoveSession(player),
	  is_started_(false),
	  cli_use_time_(0)
{
}

PStopMoveSession::~PStopMoveSession()
{
}

void PStopMoveSession::SetStopMoveInfo(int32_t seq, uint16_t speed, uint8_t mode, 
		                               A2DVECTOR& pos, uint8_t dir, int32_t use_time)
{
	seq_from_client_ = seq;
	speed_           = speed; // 百分数
	mode_            = mode;
	cli_cur_pos_     = pos;
	cli_use_time_    = use_time;
	dir_             = dir;
}

bool PStopMoveSession::StartSession(const ActiveSession* next_ses)
{
	A2DVECTOR old_pos = pplayer_->pos();
	if (mv_ctrl_.ExecuteMovement(seq_from_client_, old_pos, cli_cur_pos_, cli_use_time_))
	{
		is_started_ = true;
		// 注意speed是一个百分数
		pplayer_->sender()->MoveStop(cli_cur_pos_, speed_, mode_, dir_);
		// 修改朝向
		pplayer_->set_dir(dir_);
        MOVE_PRINTF("MMMM STOP MMM: (%6.2f, %6.2f) ----------> (%6.2f, %6.2f) ", old_pos.x, old_pos.y, 
                pplayer_->pos().x, pplayer_->pos().y);

		int tick = 0;
		if (!mv_ctrl_.CheckSeqValid(session_id()))
		{
			// 不是正常的move，使用通常的方式
			mv_ctrl_.SetSessionStartInfo(-1, -1);
			tick = millisecond_to_tick(cli_use_time_);
			if (tick < 8) tick = 8;
		}
		else
		{
			///
			/// 计算延迟
			int64_t end = g_timer->GetSysTimeMsecs();
			int svr_use_time = end - mv_ctrl_.start_time();
			int delta_t = svr_use_time - cli_use_time_;

			if (delta_t > 0) {
				return false;
			}
			if (delta_t < -kMaxMoveUseTime) {
				delta_t = -kMaxMoveUseTime;
			}

			tick = -delta_t;
			tick = millisecond_to_tick(tick);
			if (tick <= 0) {
				return false;
			}
			mv_ctrl_.SetSessionStartInfo(mv_ctrl_.GetCurMoveSeq(), session_id());
		}
		
		AutoSetTimer(tick, 1);
		return true;
	}

	return false;
}

bool PStopMoveSession::EndSession()
{
	ASSERT(session_id() != -1)

	if (is_started_) 
	{
		mv_ctrl_.SetSessionStartInfo(-1, -1);
	}
	return true;
}

} // namespace gamed
