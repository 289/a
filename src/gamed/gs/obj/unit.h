#ifndef GAMED_GS_OBJ_UNIT_H_
#define GAMED_GS_OBJ_UNIT_H_

#include "object.h"
#include "actsession.h"


namespace gamed {

class ActiveSession;
class Unit : public WorldObject
{
public:
	Unit();
	virtual ~Unit();

	virtual void Release();

	void   AddSessionAndStart(ActiveSession* ses);
	bool   HasSession();
	void   ClearSessions();
	void   ClearNextSessions();

	inline int32_t GetNextSessionID();


protected:
	int    MessageHandler(const MSG& msg);


private:
	bool   AddSession(ActiveSession* ses);
	bool   StartSession();
	bool   EndCurSession(bool is_mutable_end = false);
	void   TerminateCurSession();

	inline bool HasNextSession();
	inline void DeleteAllSession();
	inline void DeleteSessionInList();
	inline bool IsCurSessionValid(int id);


private:
	typedef std::vector<ActiveSession*> SessionListVec;

	ActiveSession*    cur_session_;
	SessionListVec    session_list_;
	int32_t           cur_assigned_ses_id_;
};

///
/// inline func
///
inline bool Unit::HasNextSession()
{
	return !session_list_.empty();
}

inline void Unit::DeleteAllSession()
{
	TerminateCurSession();
	DeleteSessionInList();
}

inline void Unit::DeleteSessionInList()
{
	for (size_t i = 0; i < session_list_.size(); ++i)
	{
		DELETE_SET_NULL(session_list_[i]);
	}
	session_list_.clear();
}

inline int32_t Unit::GetNextSessionID()
{
	++cur_assigned_ses_id_;
	cur_assigned_ses_id_ &= 0x7FFFFFFF;
	return cur_assigned_ses_id_;
}

inline bool Unit::IsCurSessionValid(int id)
{
	int cur_id = -1;
	if (cur_session_)
	{
		cur_id = cur_session_->session_id();
		if (cur_session_->session_id() == id) 
		{
			return true;
		}
	}
	
	return false;
}

} // namespace gamed

#endif // GAMED_GS_OBJ_UNIT_H_
