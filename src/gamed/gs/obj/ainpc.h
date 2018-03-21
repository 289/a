#ifndef GAMED_GS_OBJ_AINPC_H_
#define GAMED_GS_OBJ_AINPC_H_

#include "shared/base/noncopyable.h"

#include "aiaggro.h"
#include "aipolicy.h"


namespace gamed {

///
/// AINpcObject
/// 
class AINpcObject : public AIObject
{
public:
	AINpcObject(Npc& npc);
	virtual ~AINpcObject();

	void SetAggroPolicy(AggroPolicy* aggro);
	void SetChaseRange(float min_range, float max_range);

	virtual WorldObject* GetAIOwnerPtr();

	//
	// session
	//
	virtual void AddSession(ActiveSession* psession);
	virtual void ClearSessions();
	virtual bool HasSession() const;

	///
	/// common op
	///
	virtual bool CanRest();
	virtual bool QueryTarget(const XID& xid, target_info& info);
	virtual void GetPatrolPos(A2DVECTOR& pos) const;
	virtual void GetSelfPos(A2DVECTOR& pos) const;
	virtual void ReturnHome(const A2DVECTOR& target);
	virtual void SetGoingHome(AIObject::GoHameType go_type);
	virtual bool IsGoingHome() const;
    virtual void ReachDestination(const A2DVECTOR& pos);

	//
	// aggro op
	//
	virtual size_t GetAggroCount() const;	
	virtual bool   GetFirstAggro(XID& id) const;
	virtual bool   CanAggroWatch() const;
	virtual void   SetAggroWatch(bool bRst);
	virtual void   HateTarget(const XID& target);
	virtual void   RemoveAggroEntry(const XID& xid);
	virtual void   ClearAggro();

	//
	// combat op
	//
	virtual void  TriggerCombat(const XID& target);

	//
	// aggressive monster op
	//
	virtual void  GetChaseRange(float& min_range, float& max_range) const;


private:
	Npc&         npc_;
	AggroPolicy* paggro_;
	float        min_chase_range_;
	float        max_chase_range_;
	bool         is_going_home_;
};


///
/// NpcAI
///
class NpcAI : shared::noncopyable
{
public:
	NpcAI(Npc& npc);
	~NpcAI();

	bool Init(const AggroParam& aggro_param);
	void Heartbeat();
	void GoHome();
    void ResetPolicy();

	inline void SessionStart(int task_id, int session_id);
	inline void SessionEnd(int task_id, int session_id, int retcode);

	// aggro func
	bool   ChangeAggroPolicy(const AggroParam& aggro_param);
	bool   AggroWatch(const XID& target, const A2DVECTOR& pos, int faction);
	bool   ChangeWatch(const XID& target, const A2DVECTOR& pos, int faction);
	bool   CanAggroWatch() const;
	XID    GetFirstAggro() const;
	void   ClearAggro();

	// monster
	void   SetAggressiveMode(bool bRst); // 设置主动、被动
	void   SetChaseRange(float min_range, float max_range);
	void   UnderAttack(const XID& attacker);
	void   RelieveAttack();

	// service npc
	void   SomeoneGreeting(const XID& someone);
	

private:
	AINpcObject  obj_;
	AIPolicy     core_;
	AggroPolicy  aggro_;
};

/// 
/// inline func
///
inline void NpcAI::SessionStart(int task_id, int session_id)
{
	core_.SessionStart(task_id, session_id);	
}
	
inline void NpcAI::SessionEnd(int task_id, int session_id, int retcode)
{
	core_.SessionEnd(task_id, session_id, retcode);
}

} // namespace gamed

#endif // GAMED_GS_OBJ_AINPC_H_
