#ifndef GAMED_GS_OBJ_ACTSESSION_H_
#define GAMED_GS_OBJ_ACTSESSION_H_

#include "gs/global/timed_task.h"

namespace gamed {

class Unit;
class ActiveSession
{
public:
	enum SessionMask
	{
		AS_MASK_MOVE          = 0x0001,
		AS_MASK_FOLLOW_TARGET = 0x0002,
		AS_MASK_USE_ITEM      = 0x0004,
		AS_MASK_STAND_STILL   = 0x0008,
		AS_MASK_GATHER_RESULT = 0x0010,
	};

	explicit ActiveSession(Unit* obj);
	virtual ~ActiveSession();

	// 以下Proc函数是为了在session的父类有统一的入口，可以做一些统一的处理
	bool StartSessionProc(const ActiveSession* next_ses);
	bool EndSessionProc(bool is_mutable_end);
	bool RepeatSessionProc(int times);     // 重复session, times剩余次数
	bool TerminateSessionProc();           // session被强制中断

	virtual int  GetMask() const = 0;      // 自己的mask，表明自己在list中的身份，有持续时间的session这函数都要有值
	virtual int  GetExclusiveMask() = 0;   // 设置排他的mask，表明会排除list里的哪些session

	virtual bool Mutable(const ActiveSession* next_ses) { return false; }
	virtual int  GetMoveTime() const { return -1; } // 只有player move会使用该接口

	inline int64_t start_tick() const;
	inline void    set_start_tick(int64_t tick_index);
	inline int32_t session_id() const;


protected:
	// StartSession里如果没有调用AutoSetTimer()则应该return false
	virtual bool StartSession(const ActiveSession* next_ses) = 0;
	virtual bool EndSession() = 0;             // 只有已经start了的session（session_id_不等于-1）才会调用EndSession
	virtual bool RepeatSession(int times) = 0; // 重复session
	virtual bool TerminateSession() = 0;       // session被强制中断

	void    AutoSetTimer(int interval, int times, int start_time = 0);


private:
	void    SetTimer(int interval, int times, int start_time);
	void    CancelTimer();
	inline void TimerRelease();


private:
	Unit*           object_;
	TimedTaskInfo   timed_info_;

	int64_t    start_tick_; // session开始时的tick数
	int32_t    session_id_;
};

///
/// inline func
///
inline void ActiveSession::set_start_tick(int64_t tick_index)
{
	start_tick_ = tick_index;
}

inline int64_t ActiveSession::start_tick() const
{
	return start_tick_;
}

inline int32_t ActiveSession::session_id() const
{
	return session_id_;
}

inline void ActiveSession::TimerRelease()
{
	timed_info_.Release();
}

} // namespace gamed

#endif // GAMED_GS_OBJ_ACTSESSION_H_
