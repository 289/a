#ifndef GAMED_GS_MOVEMENT_PLAYER_MOVE_CTRL_H_
#define GAMED_GS_MOVEMENT_PLAYER_MOVE_CTRL_H_

#include <stdint.h>

#include "gs/global/math_types.h"


namespace gamed {

class Player;
class PlayerMoveControl
{
public:
	enum MoveModeDef
	{
		MV_MODE_NORMAL = 0,
	};

	PlayerMoveControl(Player& player);
	~PlayerMoveControl();
	
	//void    StartMoveProc(A2DVECTOR& dest, uint16_t speed, uint8_t mode);
	/*
	void    MoveContinueProc(int32_t seq, 
			                 uint16_t speed, 
							 uint8_t mode,
							 A2DVECTOR& cur_pos,
							 A2DVECTOR& dest, 
							 int32_t use_time);
							 */

	/*
	void    StopMoveProc(int32_t seq, 
			             uint16_t speed, 
						 uint8_t mode,
						 A2DVECTOR& pos, 
						 uint8_t dir,
						 int32_t use_time);
						 */

	bool    ExecuteMovement(int seq_from_cli, const A2DVECTOR& old_pos, 
			                const A2DVECTOR& cli_cur_pos, int cli_use_time);

	bool    StepTo(const A2DVECTOR& newpos);

	void    PullBackToValidPos(int seq_from_cli, const A2DVECTOR& old_pos);

	inline int       GetCurMoveSeq() const;
	inline bool      CheckSeqValid(int ses_id) const;
	inline void      SetSessionStartInfo(int seq, int ses_id);
	inline void      ResetCurCheckSeq();
	inline int       ses_start_seq() const;

	inline void      set_start_time(int64_t msecs);
	inline int64_t   start_time() const;

	//inline int32_t   cli_use_time() const;


protected:
	inline bool  CheckCmdSeq(int seq_from_cli);
	inline void  SetNextMoveSeq(int seq);
	inline int   NextMoveSeq();

	bool  CheckPlayerMove(int seq_from_cli, const A2DVECTOR& old_pos, const A2DVECTOR& cli_cur_pos, int cli_use_time);
	int   CheckPlayerSpeed(const A2DVECTOR& old_pos, const A2DVECTOR& cli_cur_pos, int cli_use_time);
	bool  PhaseControl(const A2DVECTOR& cli_cur_pos);
	bool  StepMove(const A2DVECTOR& old, const A2DVECTOR& cur_pos);


private:
	struct MoveChecker
	{
		MoveChecker() : error_counter(0) { }
		int error_counter;
	};
	MoveChecker  move_checker_;

	inline int   DecMoveCheckerError(int offset);
	inline int   IncMoveCheckerError(int offset);
	inline void  ClrMoveCheckerError();
	

private:
	Player&      player_;

	int64_t      start_time_;        // 单位毫秒millisecond
	int32_t      ses_start_seq_;     // session里检查seq时使用
	int32_t      ses_start_sessionid_;
	int32_t      cur_check_seq_;

	float        speed_ctrl_factor_; // 限制速度的统计因子，意义是当前最近用过的最大合法速度
	uint16_t     move_cmd_seq_;

	///
	///  ---- 客户端发过来的协议字段保存在以下member ----
	///
	// StartMove info
	/*
	A2DVECTOR    dest_;              // destination, also use in MoveContinue and StopMove, 该值用于客户端预测
	int32_t      start_seq_;
	int32_t      mode_;
	*/

	// MoveContinue info && StopMove info
	/*
	int32_t      seq_from_client_;
	float        speed_;
	A2DVECTOR    cli_cur_pos_;       // 客户端当前player所在的位置，server的player.pos是玩家的上一个位置
	int32_t      cli_use_time_;      // 客户端移动到该位置(cli_cur_pos_)所用时间（表示已经使用的时间）,单位为毫秒
	*/

	// StopMove info
	//uint8_t      dir_;             // direction
};

///
/// inline func
///
inline bool PlayerMoveControl::CheckSeqValid(int ses_id) const
{
	if ( (ses_start_seq_ < 0 || ses_start_seq_ != cur_check_seq_)
	  || (ses_start_sessionid_ < 0 || ses_id - ses_start_sessionid_ >= 2))
		return false;

	return true;
}

inline void PlayerMoveControl::SetSessionStartInfo(int seq, int ses_id)
{
	ses_start_seq_       = seq;
	ses_start_sessionid_ = ses_id;
}

inline int PlayerMoveControl::ses_start_seq() const
{
	return ses_start_seq_;
}

inline void PlayerMoveControl::ResetCurCheckSeq()
{
	cur_check_seq_ = -1;
}

inline int PlayerMoveControl::GetCurMoveSeq() const
{
	return move_cmd_seq_;
}

inline int PlayerMoveControl::NextMoveSeq()
{
	move_cmd_seq_ = (move_cmd_seq_ + 1 ) & 0xFFFF;
	return move_cmd_seq_;
}

inline void PlayerMoveControl::SetNextMoveSeq(int seq)
{
	move_cmd_seq_ = seq;
}

inline bool PlayerMoveControl::CheckCmdSeq(int seq_from_cli)
{
	int seq  = GetCurMoveSeq();
	cur_check_seq_ = seq;
	int seq_offset = (seq_from_cli - seq) & 0xFFFF;
	if (seq_from_cli != -1 && seq_offset > 2000)
	{
		return false;
	}

	if (seq_from_cli != -1 && seq_offset)
	{
		SetNextMoveSeq(seq_from_cli);
	}
	else
	{
		// 序列号进一
		NextMoveSeq();
	}

	return true;
}

inline void PlayerMoveControl::set_start_time(int64_t msecs)
{
	start_time_ = msecs;
}

inline int64_t PlayerMoveControl::start_time() const
{
	return start_time_;
}

inline int PlayerMoveControl::DecMoveCheckerError(int offset)
{
	if ((move_checker_.error_counter -= offset) < 0)
	{
		move_checker_.error_counter = 0;
	}
	return  move_checker_.error_counter;
}

inline int PlayerMoveControl::IncMoveCheckerError(int offset)
{
	move_checker_.error_counter += offset;
	return move_checker_.error_counter;
}

inline void PlayerMoveControl::ClrMoveCheckerError()
{
	move_checker_.error_counter = 0;
}


/*
inline int32_t PlayerMoveControl::cli_use_time() const 
{
	return cli_use_time_;
}
*/

} // namespace gamed

#endif // GAMED_GS_MOVEMENT_PLAYER_MOVE_CTRL_H_
