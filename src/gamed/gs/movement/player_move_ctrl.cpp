#include "player_move_ctrl.h"

#include "game_module/pathfinder/include/path_finder.h"

#include "gs/global/game_util.h"
#include "gs/global/game_def.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"

#include "gs/scene/grid.h"
#include "gs/scene/world.h"

#include "gs/player/player.h"
#include "gs/player/player_ctrl.h"
#include "gs/player/player_sender.h"

#include "pmv_session.h"
#include "move_util.h"


namespace gamed {

static const int kMoveFaultTolerantValue = 10; // 单位：次

PlayerMoveControl::PlayerMoveControl(Player& player)
		: player_(player),
		  ses_start_seq_(-1),
		  ses_start_sessionid_(-1),
		  cur_check_seq_(-1),
		  speed_ctrl_factor_(16.f),
		  move_cmd_seq_(0)
{ 
}

PlayerMoveControl::~PlayerMoveControl()
{
}

/*
void PlayerMoveControl::StartMoveProc(A2DVECTOR& dest, uint16_t speed, uint8_t mode)
{
	dest_          = dest;
	start_seq_     = GetCurMoveSeq();
	speed_         = ((float)speed) / 100.f;  // 百分数
	mode_          = mode;

	player_.sender()->MoveStart(dest, speed, mode);
	__PRINTF("MMMM START MMM: (%6.2f, %6.2f) ", dest_.x, dest_.y);
	player_.AddSessionAndStart(new PStartMoveSession(&player_));
}
*/

/*
void PlayerMoveControl::MoveContinueProc(int32_t seq, 
		                                 uint16_t speed, 
										 uint8_t mode,
										 A2DVECTOR& cur_pos,
										 A2DVECTOR& dest, 
										 int32_t use_time)
{
	seq_from_client_ = seq;
	speed_           = ((float)speed) / 100.f;  // 百分数
	mode_            = mode;
	cli_cur_pos_     = cur_pos;
	dest_            = dest;
	cli_use_time_    = use_time;

	A2DVECTOR old_pos = player_.pos();
	if (ExecuteMovement(old_pos))
	{
		// 注意speed是一个百分数
		player_.sender()->Move(cur_pos, dest, speed, mode);
		__PRINTF("MMMM MOVE MMM: (%6.2f, %6.2f) ----------> (%6.2f, %6.2f) ", old_pos.x, old_pos.y, player_.pos().x, player_.pos().y);
		player_.AddSessionAndStart(new PMoveSession(&player_));
	}
}
*/

/*
void PlayerMoveControl::StopMoveProc(int32_t seq, 
		                             uint16_t speed, 
									 uint8_t mode,
									 A2DVECTOR& pos, 
									 uint8_t dir,
									 int32_t use_time)
{
	seq_from_client_ = seq;
	speed_           = ((float)speed) / 100.f;  // 百分数
	mode_            = mode;
	cli_cur_pos_     = pos;
	cli_use_time_    = use_time;

	A2DVECTOR old_pos = player_.pos();
	if (ExecuteMovement(old_pos))
	{
		// 注意speed是一个百分数
		player_.sender()->MoveStop(pos, speed, mode, dir);
		__PRINTF("MMMM STOP MMM: (%6.2f, %6.2f) ----------> (%6.2f, %6.2f) ", old_pos.x, old_pos.y, player_.pos().x, player_.pos().y);
		player_.AddSessionAndStart(new PStopMoveSession(&player_));
	}
}
*/

bool PlayerMoveControl::ExecuteMovement(int seq_from_cli, const A2DVECTOR& old_pos, 
		                                const A2DVECTOR& cli_cur_pos, int cli_use_time)
{
	if (!CheckCmdSeq(seq_from_cli))
	{
		__PRINTF("move seq_num check error! return false!");
        PullBackToValidPos(seq_from_cli, old_pos);
		return false;
	}

	if (cli_use_time < kMinMoveUseTime) 
	{
		MOVE_PRINTF("move cli_use_time:%d less than kMinMoveUseTime", cli_use_time);
		cli_use_time = kMinMoveUseTime;
	}

	if (!CheckPlayerMove(seq_from_cli, old_pos, cli_cur_pos, cli_use_time))
	{
		//__PRINTF("用户%ld移动没有通过检查! CheckPlayerMove() failed!", player_.role_id());
		return false;
	}
	
	if (!StepMove(old_pos, cli_cur_pos))
	{
		__PRINTF("用户%ld移动的位置(%6.2f, %6.2f)超出管辖范围", player_.role_id(), cli_cur_pos.x, cli_cur_pos.y);
		return false;
	}

	return true;
}


bool PlayerMoveControl::CheckPlayerMove(int seq_from_cli, const A2DVECTOR& old_pos, const A2DVECTOR& cli_cur_pos, int cli_use_time)
{
	int rst = CheckPlayerSpeed(old_pos, cli_cur_pos, cli_use_time);
	if (rst >= 0)
	{
		// 速度检查基本通过，进行通过图或碰撞检测
		if (PhaseControl(cli_cur_pos))
		{
			if (rst == 0)
			{
				// 检测通过还要判断move checker
				if (DecMoveCheckerError(1) < kMoveFaultTolerantValue)
					return true;

				__PRINTF("用户%ld的移动累积Error还是超过容忍值, error=%d次", 
						player_.role_id(), move_checker_.error_counter);
			}
			else
			{
				// rst大于0，速度检查有问题，做一次累积error
				// 通过图检查通过，可以返回true，进行一次StepMove
				if (IncMoveCheckerError(1) < kMoveFaultTolerantValue)
					return true;

				__PRINTF("用户%ld的移动累积Error超过容忍值=%d", player_.role_id(), 
						kMoveFaultTolerantValue);
			}
		}
		else
		{
			// 通过图上cli_cur_pos点是不可达点，因此返回false不进行StepMove()
			// 只有连续出错超过10次才拉回有效位置，这里做了个容错
			//if (IncMoveCheckerError(1) < kMoveFaultTolerantValue)
				//return false;

			__PRINTF("用户%ld的可疑移动数据(%f, %f)在通过图上是不可达点 old_pos:(%f, %f)", 
					player_.role_id(), cli_cur_pos.x, cli_cur_pos.y, old_pos.x, old_pos.y);
		}
	}

	///
	/// pull back to old_pos
	///
	__PRINTF("用户%ld的pos位置被强行同步至(%6.2f, %6.2f)", player_.role_id(), old_pos.x, old_pos.y);

	// ???????? Glog
	
	// 清除现在的移动错误计算	
	ClrMoveCheckerError();

	// 修正新的命令序号
	int seq = (seq_from_cli + 100) & 0xFFFF;
	SetNextMoveSeq(seq);

	// 考虑重新发送玩家的速度数据
	player_.sender()->NotifyPlayerMoveProp(player_.cur_speed());
	// 将玩家拉回原来位置，并广播给视野里其他玩家
	player_.sender()->PullBackToValidPos(seq, old_pos);
	player_.sender()->BroadcastPlayerPullBack(old_pos);
	// 清空后面所有的session
	player_.ClearNextSessions();
	return false;
}

void PlayerMoveControl::PullBackToValidPos(int seq_from_cli, const A2DVECTOR& old_pos)
{
	// 修正新的命令序号
	int seq = (seq_from_cli + 100) & 0xFFFF;
	SetNextMoveSeq(seq);

	// 拉回到原来位置
	player_.sender()->PullBackToValidPos(seq, old_pos);
}

int PlayerMoveControl::CheckPlayerSpeed(const A2DVECTOR& old_pos, const A2DVECTOR& cli_cur_pos, int cli_use_time)
{
	if (cli_use_time < kMinMoveUseTime || cli_use_time > kMaxMoveUseTime)
	{
		return -1;
	}

	A2DVECTOR offset   = cli_cur_pos;
	offset -= old_pos;

	int move_mode      = 0;
	float t            = cli_use_time * 0.001f;
	float speed_square = 10000.f;
	float cur_speed    = (player_.GetSpeedByMode(move_mode) + 0.2f) * t; // 加有少量的允许值
	float max_speed    = (kMaxMoveSpeed+0.2f) * (kMaxMoveSpeed+0.3f) * (t*t);

	// 普通移动
	if (MV_MODE_NORMAL == move_mode)
	{
		speed_square = offset.x * offset.x + offset.y * offset.y;
	}
	else
	{
		speed_square = offset.x * offset.x + offset.y * offset.y;
	}

	// 检查最大速度
	if (speed_square > max_speed)
	{
		// 超过最大速度
		__PRINTF("用户%ld超过最大速度,speed_square:%f max_speed:%f t:%f cli_cur_pos:(%f, %f)", 
				player_.role_id(), speed_square, max_speed, t, cli_cur_pos.x, cli_cur_pos.y);
		return -1;
	}

	float cur_speed_square = cur_speed * cur_speed;
	float stmp = 1.f / (t*t);
	if (speed_square > cur_speed_square)
	{
		// 如果当前速度超过最大速度，那么和历史中使用过的最大速度进行比较
		// 2014.02.28增加到1.5倍的容忍度
		// 2014.09.01增加到2.5倍的容忍度
		if (speed_square * stmp > speed_ctrl_factor_ * 2.5)	
		{
			__PRINTF("用户%ld超过历史最大速度,cur_speed_square:%f speed_square:%f stmp:%f factor:%f t:%f move_cmd_seq:%d", 
					player_.role_id(), cur_speed_square, speed_square, stmp, speed_ctrl_factor_, t, move_cmd_seq_);
			return -1;
		}

		return 1;
	}
	else
	{
		// 记录上一次移动的速度
		speed_ctrl_factor_ = cur_speed_square * stmp;
	}

	return 0;
}

bool PlayerMoveControl::PhaseControl(const A2DVECTOR& cli_cur_pos)
{
	return pathfinder::IsWalkable(player_.world_id(), A2D_TO_PF(cli_cur_pos));
}

bool PlayerMoveControl::StepTo(const A2DVECTOR& newpos)
{
	return StepMove(player_.pos(), newpos);
}

bool PlayerMoveControl::StepMove(const A2DVECTOR& old, const A2DVECTOR& cur_pos)
{
	A2DVECTOR newpos(cur_pos), oldpos(old);

	World* pworld = player_.world_plane();
	if (pworld->IsOutsideGrid(newpos.x, newpos.y))
	{
		return false;
	}

	if (!pworld->IsInGridLocal(newpos.x, newpos.y))
	{
		// 超出边界
		return false;
	}

	//__PRINTF("StepMove old_pos:(%f,%f) -> new_pos:(%f,%f)", old.x, old.y, cur_pos.x, cur_pos.y);
	player_.set_pos(newpos);
	player_.world_plane()->UpdateAOI(player_.object_xid(), newpos);

	return true;
}

} // namespace gamed
