#include "unit.h"

#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"

#include "gs/global/game_def.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"

#include "actsession.h"


namespace gamed {

Unit::Unit()
	: cur_session_(NULL),
	  cur_assigned_ses_id_(0)
{
}

Unit::~Unit()
{
	SAFE_DELETE(cur_session_);
	DeleteSessionInList();
}

void Unit::Release()
{
	SAFE_DELETE(cur_session_);
	DeleteSessionInList();
	WorldObject::Release();
}

void Unit::AddSessionAndStart(ActiveSession* ses)
{
	if (AddSession(ses)) StartSession();
}

bool Unit::AddSession(ActiveSession* ses)
{
	int exclusive_mask = ses->GetExclusiveMask();
	if (session_list_.size() >= (size_t)kMaxUnitSessions)
	{
		LOG_WARN << "Unit对象的session队列满，清空原有队列内容 XID(" << object_xid().id 
			<< ", " << object_xid().type << ")";
		DeleteSessionInList();

		if (cur_session_)
		{
			// 如果当前session存在，检查当前session的执行时间，如果过长，则清除之
			int64_t tick_count = g_timer->get_tick() - cur_session_->start_tick();
			if (tick_count > kSessionTimeout * TICK_PER_SEC)
			{
				LOG_ERROR << "Unit id:" << object_xid().id << " type:" << object_xid().type
					<< " Session执行时间过长而超时session mask:" << cur_session_->GetMask();
				TerminateCurSession();
			}
		}
	}

	if (exclusive_mask)
	{
		SessionListVec::reverse_iterator rit = session_list_.rbegin();
		for (; rit != session_list_.rend(); )
		{
			ActiveSession* ses_ptr = *rit;
			if ((ses_ptr->GetMask() & exclusive_mask) || ses_ptr->Mutable(NULL))
			{
				__PRINTF("删除了被排斥的session %x", ses_ptr->GetMask());
				DELETE_SET_NULL(ses_ptr);
				rit = SessionListVec::reverse_iterator(session_list_.erase((++rit).base()));
			}
			else
			{
				++rit;
			}
		}
	}

	if (session_list_.empty() && cur_session_)
	{
		if (cur_session_->Mutable(ses))
		{
			EndCurSession(true);
		}
	}

	session_list_.push_back(ses);
	return !cur_session_;
}

bool Unit::StartSession()
{
	if (cur_session_ != NULL) return false;
	bool rst = false;
	while (cur_session_ == NULL && HasNextSession())
	{
		cur_session_ = session_list_[0];
		session_list_.erase(session_list_.begin());
		ActiveSession* next_ses = HasNextSession() ? session_list_[0] : NULL;
		if (!(rst = cur_session_->StartSessionProc(next_ses)))
		{
			EndCurSession();
		}
		else
		{
			cur_session_->set_start_tick(g_timer->get_tick());
		}
	}
	return rst;
}

bool Unit::EndCurSession(bool is_mutable_end)
{
	if (cur_session_ == NULL) return false;
	cur_session_->EndSessionProc(is_mutable_end);
	DELETE_SET_NULL(cur_session_);
	return true;
}

void Unit::TerminateCurSession()
{
	if (cur_session_)
	{
		cur_session_->TerminateSessionProc();
		DELETE_SET_NULL(cur_session_);
	}
}

bool Unit::HasSession()
{
	return cur_session_ != NULL || HasNextSession();
}

void Unit::ClearSessions()
{
	DeleteAllSession();
}

void Unit::ClearNextSessions()
{
	DeleteSessionInList();
}

int Unit::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_OBJ_SESSION_END:
			{
				if (!IsCurSessionValid(msg.param)) 
				{
					break;
				}
				EndCurSession();
				StartSession();
			}
			break;

		case GS_MSG_OBJ_SESSION_REPEAT:
			{
                int32_t times = high32bits(msg.param);
                int32_t param = low32bits(msg.param);
				if (!IsCurSessionValid(param)) 
				{
					break;
				}
				if (!cur_session_->RepeatSessionProc(times))
				{
					EndCurSession();
					StartSession();
				}
			}
			break;

		default:
			if (WorldObject::MessageHandler(msg) != 0)
			{
				return -1;
			}
			return 0;
	}

	return 0;
}

} // namespace gamed
