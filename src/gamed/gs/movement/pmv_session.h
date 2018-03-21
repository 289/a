#ifndef GAMED_GS_MOVEMENT_PMV_SESSION_H_
#define GAMED_GS_MOVEMENT_PMV_SESSION_H_

#include "gs/player/psession.h"
#include "gs/global/math_types.h"


namespace gamed {

class PlayerMoveControl;

///
/// Start move
///
class PStartMoveSession : public PlayerSession
{
public:
	explicit PStartMoveSession(Player* player);
	virtual ~PStartMoveSession();

	void SetStartMoveInfo(A2DVECTOR& dest, uint16_t speed, uint8_t mode);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();

	virtual bool Mutable(const ActiveSession* next_ses);
	virtual int  GetMask() const;
	virtual int  GetExclusiveMask();

protected:
	PlayerMoveControl&  mv_ctrl_;
	int    base_time_;

private:
	// recv from client
	A2DVECTOR    dest_;   // 目标点，用于客户端的预测（跟随）
	uint16_t     speed_;
	uint8_t      mode_;
};

///
/// Move Continue
///    （1）unit.cpp调用基类if (cur_session_->Mutable(ses))，当cur_session_是PMoveSession时:
///         PMoveSession的Mutable函数用的是PStartMoveSession里的，因为没重载。
///
class PMoveSession : public PStartMoveSession
{
public:
	explicit PMoveSession(Player* player);
	virtual ~PMoveSession();

	void SetMoveInfo(int32_t seq, uint16_t speed, uint8_t mode,
					 A2DVECTOR& cur_pos, A2DVECTOR& dest, int32_t use_time);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();
	virtual int  GetMoveTime() const;

private:
	int    CalculateTimeDelay(const ActiveSession* next_ses);

private:
	// recv from client
	int32_t      seq_from_client_;
	uint16_t     speed_;
	uint8_t      mode_;
	A2DVECTOR    cli_cur_pos_;  // 客户端当前player所在的位置，server的player.pos是玩家的上一个位置
	A2DVECTOR    dest_;         // 目标点，用于客户端的预测（跟随）
	int32_t      cli_use_time_; // 客户端移动到该位置(cli_cur_pos_)所用时间（表示已经使用的时间）,单位为毫秒
};

///
/// Stop Move
///
class PStopMoveSession : public PMoveSession
{
public:
	explicit PStopMoveSession(Player* player);
	virtual ~PStopMoveSession();

	void SetStopMoveInfo(int32_t seq, uint16_t speed, uint8_t mode, A2DVECTOR& pos, 
						 uint8_t dir, int32_t use_time);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();
	virtual bool Mutable(const ActiveSession* next_ses) { return false; }

private:
	bool         is_started_;
	// recv from client
	int32_t      seq_from_client_;
	uint16_t     speed_;
	uint8_t      mode_;
	A2DVECTOR    cli_cur_pos_;  // player当前停止的位置点
	int32_t      cli_use_time_; // 客户端移动到该位置(cli_cur_pos_)所用时间（表示已经使用的时间）,单位为毫秒
	uint8_t      dir_;          // 朝向
};

} // namespace gamed

#endif // GAMED_GS_MOVEMENT_PMV_SESSION_H_
