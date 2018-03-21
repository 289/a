#ifndef GAMED_GS_MOVEMENT_NPC_MVSESSION_H_
#define GAMED_GS_MOVEMENT_NPC_MVSESSION_H_

#include "gs/obj/npcsession.h"

#include "gs/global/math_types.h"


namespace pathfinder {
	class NpcRamble;
	class NpcChase;
} // namespace pathfinder


namespace gamed {

///
/// NpcStrollSession 游荡，闲逛
///
class NpcStrollSession : public NpcSession
{
	static const int kMaxStrollTimes;   // 单位：次
public:
	explicit NpcStrollSession(Npc* npc);
	virtual ~NpcStrollSession();

	virtual bool OnStartSession(const ActiveSession* next_ses);
	virtual int  OnEndSession();
	virtual bool RepeatSession(int times);

	void SetTarget(const A2DVECTOR& center, int timeout, float range);


private:
	bool  Run();
	float GetSpeed(); // 单位：米每秒


private:
	A2DVECTOR  center_;
	int        timeout_;
	float      range_;
	bool       stop_flag_;
	uint16_t   speed_to_client_;  // 厘米每秒，发给客户端用
	pathfinder::NpcRamble* agent_;
};


///
/// NpcFollowTargetSession 跟随，追随
///
class NpcFollowTargetSession : public NpcSession
{
	static const int kMaxFollowTimes;   // 单位：次
public:
	explicit NpcFollowTargetSession(Npc* npc);
	virtual ~NpcFollowTargetSession();

	virtual bool OnStartSession(const ActiveSession* next_ses);
	virtual int  OnEndSession();
	virtual bool RepeatSession(int times);

	virtual int GetMask() const    { return AS_MASK_FOLLOW_TARGET; }
	virtual int GetExclusiveMask() { return ~(AS_MASK_FOLLOW_TARGET|AS_MASK_MOVE); }

	void SetTarget(const XID& target, float min_range, float max_range, int timeout);


private:
	bool  Run();
	float GetSpeed(); // 单位：米每秒
	void  TrySendStop();


private:
	XID       target_;
	float     min_range_squared_;
	float     max_range_squared_;
	float     range_target_;
	int       timeout_;
	uint16_t  speed_to_client_;  // 厘米每秒，发给客户端用
	bool      stop_flag_;
	A2DVECTOR self_start_pos_;
	char      reachable_count_;
	int       retcode_;
	pathfinder::NpcChase* agent_;
};


///
/// NpcPatrolSession: 巡逻等，用于到达某个点
///
class NpcPatrolSession : public NpcSession
{
	static const int kMaxPatrolTimes = 100;   // 单位：次
public:
	explicit NpcPatrolSession(Npc* npc);
	virtual ~NpcPatrolSession();

	virtual bool OnStartSession(const ActiveSession* next_ses);
	virtual int  OnEndSession();
	virtual bool RepeatSession(int times);

	virtual int GetMask() const    { return AS_MASK_STAND_STILL; }
	virtual int GetExclusiveMask() { return ~AS_MASK_STAND_STILL; }

	void SetTarget(const A2DVECTOR& target, int timeout, float range);


private:
	bool  Run();
	float GetSpeed(); // 单位：米每秒
	void  TrySendStop();


private:
	A2DVECTOR target_pos_;
	float     range_target_;
	float     range_squared_;
	int       timeout_;
	uint16_t  speed_to_client_;  // 厘米每秒，发给客户端用
	bool      stop_flag_;
	char      reachable_count_;
	int       retcode_;

	pathfinder::NpcChase* agent_;
};

} // namespace gamed

#endif // GAMED_GS_MOVEMENT_NPC_MVSESSION_H_
