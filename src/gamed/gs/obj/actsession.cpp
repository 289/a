#include "actsession.h"

#include "unit.h"


namespace gamed {

ActiveSession::ActiveSession(Unit* obj)
	: object_(obj),
	  start_tick_(-1),
	  session_id_(-1)
{
}

ActiveSession::~ActiveSession()
{
	CancelTimer();

	start_tick_ = -1;
	session_id_ = -1;
}

void ActiveSession::SetTimer(int interval, int times, int start_time)
{
	timed_info_.xid        = object_->object_xid();
	timed_info_.interval   = interval;
	timed_info_.times      = times;
	timed_info_.start_time = start_time;
	timed_info_.param      = session_id_;

	s_pTimedTask->AddTimedTask(timed_info_);
}
	
void ActiveSession::CancelTimer()
{
	if (timed_info_.HasBeenTimed())
	{
		s_pTimedTask->Cancel(timed_info_);
		TimerRelease();
	}
}

void ActiveSession::AutoSetTimer(int interval, int times, int start_time)
{
	if (timed_info_.HasBeenTimed())
	{
		CancelTimer();
		session_id_ = object_->GetNextSessionID();
	}
	SetTimer(interval, times, start_time);
}

bool ActiveSession::StartSessionProc(const ActiveSession* next_ses)
{
	session_id_ = object_->GetNextSessionID();
	return StartSession(next_ses);
}

bool ActiveSession::EndSessionProc(bool is_mutable_end)
{
	if (!is_mutable_end) 
		TimerRelease();
	if (session_id_ == -1)
		return false;

	bool rst = EndSession();
	return rst;
}

bool ActiveSession::RepeatSessionProc(int times)
{
	ASSERT(session_id_ != -1);
	return RepeatSession(times);
}

bool ActiveSession::TerminateSessionProc()
{
	if (session_id_ == -1)
		return false;

	bool rst = TerminateSession();
	return rst;
}

} // namespace gamed
