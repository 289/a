#ifndef GAMED_GS_PLAYER_PSESSION_H_
#define GAMED_GS_PLAYER_PSESSION_H_

#include "gs/obj/actsession.h"
#include "gs/global/math_types.h"


namespace gamed {

class Player;

///
/// PlayerSession
///
class PlayerSession : public ActiveSession
{
public:
	explicit PlayerSession(Player* player);
	virtual ~PlayerSession();

	virtual bool StartSession(const ActiveSession* next_ses) { return false; }
	virtual bool EndSession()    { return false; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession();

	virtual int GetMask() const = 0;
	virtual int GetExclusiveMask() = 0;

protected:
	Player* pplayer_;
};


///
/// PlayerEmptySession
///
class PlayerEmptySession : public PlayerSession
{
public:
	explicit PlayerEmptySession(Player* player) : PlayerSession(player) { }
	virtual ~PlayerEmptySession() { }

	// 子类可以按需重载以下虚函数
	virtual bool StartSession(const ActiveSession* next_ses) { return false; }
	virtual bool EndSession()       { return false; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return 0; }
	virtual int  GetExclusiveMask() { return ~0; }
};


///
/// PSayHelloSession
///
class PSayHelloSession : public PlayerSession
{
public:
	explicit PSayHelloSession(Player* player) : PlayerSession(player) { }
	virtual ~PSayHelloSession() { }

	void SetTarget(const XID& target) { target_ = target; }

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession()       { return true; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return AS_MASK_USE_ITEM; }
	virtual int  GetExclusiveMask() { return ~(AS_MASK_MOVE); }

private:
	XID target_;
};


///
/// PRegionTransportSession
///
class PRegionTransportSession : public PlayerSession
{
public:
	explicit PRegionTransportSession(Player* player) : PlayerSession(player) { }
	virtual ~PRegionTransportSession() { }

	void SetTransportTarget(int32_t world_id, const A2DVECTOR& pos);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession()       { return true; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return AS_MASK_MOVE; }
	virtual int  GetExclusiveMask() { return ~(AS_MASK_MOVE); }

private:
	int32_t   target_world_id_;
	A2DVECTOR target_pos_;
};


///
/// PGatherPrepareSession
///
class PGatherPrepareSession : public PlayerSession
{
public:
	explicit PGatherPrepareSession(Player* player) 
		: PlayerSession(player)
	{ }
	virtual ~PGatherPrepareSession() { }

	void SetTarget(const XID& matterxid, int32_t gather_seq_no, int32_t gather_timeout);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession()       { return false; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return AS_MASK_STAND_STILL; }
	virtual int  GetExclusiveMask() { return ~(AS_MASK_MOVE); }

private:
	XID     matter_xid_;
	int32_t gather_seq_no_;
	int32_t gather_timeout_; // 单位:s
};


///
/// PGatherSession
///
class PGatherSession : public PlayerSession
{
public:
	explicit PGatherSession(Player* player) 
		: PlayerSession(player),
		  matter_tid_(0),
		  gather_time_(50),
		  gather_flag_(false),
		  gather_seq_no_(0),
		  stop_reason_(0),
          tg_task_id_(0),
          tg_task_prob_(0),
          tg_task_interval_(0),
          tg_task_times_(0)
	{ }
	virtual ~PGatherSession() { }

	void SetTarget(const XID& matterxid, int32_t gather_time, int32_t gather_seq_no, int32_t mattertid);
    void SetTriggerTask(int32_t task_id, int32_t task_prob, int32_t task_interval, int32_t task_times);

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();
	virtual bool RepeatSession(int times);
	virtual bool TerminateSession();
	virtual bool Mutable(const ActiveSession* next_ses);

	virtual int  GetMask() const    { return AS_MASK_STAND_STILL; }
	virtual int  GetExclusiveMask() { return ~(AS_MASK_MOVE); }

private:
    void DeliverTask();

private:
	XID     matter_xid_;
	int32_t matter_tid_;       // 模板id，剧情矿需要使用
	int32_t gather_time_;      // 单位：ms
	bool    gather_flag_;
	int32_t gather_seq_no_;
	int8_t  stop_reason_;      // 参照G2C::GatherStop

    // 采矿过程中发任务
    int32_t tg_task_id_;       // 采集过程中发的任务，0表示没有任务
    int32_t tg_task_prob_;     // 采集过程中发任务的概率，万分数
    int32_t tg_task_interval_; // 采集过程中发任务的间隔，单位ms
    int32_t tg_task_times_;    // 采集过程中发任务的次数
};


///
/// PGatherResultSession
///
class PGatherResultSession : public PlayerSession
{
public:
	explicit PGatherResultSession(Player* player) 
		: PlayerSession(player)
	{ }
	virtual ~PGatherResultSession() { }

	void SetTarget(const XID& matterxid, int32_t gather_seq_no, bool is_success, int8_t reason = 0);

	virtual bool StartSession(const ActiveSession* next_ses) { return false; }
	virtual bool EndSession()       { return false; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return AS_MASK_GATHER_RESULT; }
	virtual int  GetExclusiveMask() { return ~0; }

	bool    IsGatherSuccess(const XID& matter_xid, int32_t gather_seq_no) const;
	bool    GetFailureReason() const;

private:
	XID     matter_xid_;
	int32_t gather_seq_no_;
	bool    is_success_;
	int8_t  reason_;
};

///
/// PInvincibleSession
///
class PInvincibleSession : public PlayerSession
{
public:
	PInvincibleSession(Player* player, int32_t time):
		PlayerSession(player),
		time_(time)
	{
	}

	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();
	virtual bool RepeatSession(int times)      { return false; } 
	virtual bool TerminateSession()            { return true; }
	virtual bool Mutable(const ActiveSession*) { return true; }
	virtual int  GetMask() const               { return 0xFFFFFFFF; }
	virtual int  GetExclusiveMask()            { return 0xFFFFFFFF; }

private:
	int32_t time_; //无敌Buff持续时间
};

///
/// PLogonInvincibleSession
///
class PLogonInvincibleSession : public PInvincibleSession
{
public:
	PLogonInvincibleSession(Player* player, int32_t time):
		PInvincibleSession(player, time)
	{
	}
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_PSESSION_H_
