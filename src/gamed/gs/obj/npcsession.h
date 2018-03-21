#ifndef GAMED_GS_OBJ_NPCSESSION_H_
#define GAMED_GS_OBJ_NPCSESSION_H_

#include "actsession.h"


namespace gamed {

enum
{
	NSRC_SUCCESS = 0,
	NSRC_OUT_OF_RANGE,
	NSRC_TIMEOUT,
	NSRC_ERR_PATHFINDING,
};

class Npc;
class NpcSession : public ActiveSession
{
public:
	explicit NpcSession(Npc* npc);
	virtual ~NpcSession();

	inline void SetAITask(int id);

	// 子类可以按需重载以下虚函数
	virtual bool OnStartSession(const ActiveSession* next_ses) { return false; }
	virtual int  OnEndSession() { return -1; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession();

	virtual int GetMask() const    { return 0; }
	virtual int GetExclusiveMask() { return ~0; }


private:
	// 以下虚函数子类不能重载
	virtual bool StartSession(const ActiveSession* next_ses);
	virtual bool EndSession();
	void    NotifySessionStart();
	void    NotifySessionEnd(int retcode);


protected:
	Npc*    pnpc_;
	int     ai_task_id_;
};

///
/// inline func
///
inline void NpcSession::SetAITask(int id)
{
	ai_task_id_ = id;
}


///
/// NpcEmptySession
///
class NpcEmptySession : public NpcSession
{
public:
	explicit NpcEmptySession(Npc* npc) : NpcSession(npc) { }
	virtual ~NpcEmptySession() { }

	// 子类可以按需重载以下虚函数
	virtual bool OnStartSession(const ActiveSession* next_ses) { return false; }
	virtual int  OnEndSession()     { return 0; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return 0; }
	virtual int  GetExclusiveMask() { return ~0; }
};


///
/// NpcCombatSession
///
class NpcCombatSession : public NpcSession
{
	static const int kMaxCombatRetainSecs = 3600;   // 单位：秒
public:
	explicit NpcCombatSession(Npc* npc) : NpcSession(npc) { }
	virtual ~NpcCombatSession() { }

	// 子类可以按需重载以下虚函数
	virtual bool OnStartSession(const ActiveSession* next_ses);
	virtual int  OnEndSession()     { return 0; }
	virtual bool RepeatSession(int times) { return false; } 
	virtual bool TerminateSession() { return true; }

	virtual int  GetMask() const    { return AS_MASK_STAND_STILL; }
	virtual int  GetExclusiveMask() { return ~0; }
};

} // namespace gamed

#endif // GAMED_GS_OBJ_NPCSESSION_H_
