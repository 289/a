#ifndef GAMED_GS_OBJ_NPC_H_
#define GAMED_GS_OBJ_NPC_H_

#include "gs/global/game_def.h"

#include "unit.h"
#include "npc_def.h"


namespace gamed {

class NpcImp;
class NpcAI;
class NpcSender;
class MonsterImp;

/**
 * @brief Npc
 *    （1）TODO: Npc所有的成员变量都必须在Release()里重置及释放，因为Npc指针放回对象池并没有真正delete
 */
class Npc : public Unit
{
	typedef mapDataSvr::TemplID TemplID;
	typedef mapDataSvr::ElemID  ElemID;
	typedef std::set<RoleID> InViewPlayersSet;

	///
	/// TODO: Npc的friend class只能调用Npc的public及protected成员函数
	/// 人为保证这个限制
	///
	friend class MonsterImp;
	friend class ServiceNpcImp;

public:
	Npc();
	virtual ~Npc();

	bool Init(const npcdef::NpcInitData& init_data);
	void HasInsertToWorld();

	// virtual func
	virtual void Release();
	virtual void OnHeartbeat();
	virtual int  OnDispatchMessage(const MSG& msg);
    virtual bool CanCombat() const;

	bool  StepMove(const A2DVECTOR& offset);
	void  PlayerEnterView(RoleID playerid, int32_t linkid, int32_t sid);
	void  PlayerLeaveView(RoleID playerid);
	
	// ai 相关
	bool  CanRest();
	bool  GetBirthPlace(A2DVECTOR& pos);
	void  TriggerCombat(const XID& target);
	float GetChaseSpeed() const;
    void  ReachDestination(const A2DVECTOR& pos);

	// session 相关
	void  NPCSessionStart(int task_id, int session_id);
	void  NPCSessionEnd(int task_id, int session_id, int retcode);

	// inline 
	inline NpcSender* sender();
	inline TemplID    templ_id() const;
	inline ElemID     elem_id() const;
	inline bool       is_idle();
	inline npcdef::NpcStates state() const;


protected:
	bool  CanStroll();
	void  SetNormalState();
	void  SetIdleState();
	void  SetZombieState(int time_counter);
	void  SetRebornState(int time_counter);
    void  SetRecycleState();
    bool  CheckStateSwitch(npcdef::NpcStates new_state) const;
	const InViewPlayersSet& GetPlayersInView();
    void  RebornAtOnce();
    void  SetBirthPlace(const A2DVECTOR& pos);
	
	// 只能由派生类或者imp类调用
	inline NpcAI* aicore();
	inline void   set_state(npcdef::NpcStates state);


private:
	bool InitRuntimeData();
	int  MessageHandler(const MSG& msg);
	void DriveMachine();
	bool CheckIdleState();
	bool HasPlayerInView() const;
	void LifeExhaust();
	void Reborn();
	void HandleWorldClose();
	void MapElementDeactive();
	void RecycleSelf();
	bool CanAutoReborn();
    void HandleObjectTeleport(const MSG& msg);
    void HandleMonsterMove(const MSG& msg);
    void HandleMonsterSpeed(const MSG& msg);
    bool CanTeleport();


private:
	///
	/// TODO: npc的imp类不要直接访问npc的成员变量（虽然是friend class）
	///       可以直接访问public及protected的成员函数。
	///
	ElemID       elem_id_;
	TemplID      templ_id_;

	NpcImp*      pimp_;
	NpcAI*       ai_core_;
	NpcSender*   sender_;

	int          stroll_timer_;
	int          idle_timer_;
	int          zombie_timer_;
	int          reborn_timer_;

	InViewPlayersSet   in_view_players_set_;

	npcdef::NpcStates  state_;
	A2DVECTOR    birth_place_;
};


/**
 * @brief NpcImp
 *    （1）不要直接访问Npc的成员变量（虽然是friend class）
 *         可以通过调用Npc的public及protected函数来实现
 */
class NpcImp
{
public:
	NpcImp(Npc& npc) 
		: npc_(npc), faction_(0), enemy_faction_(0) 
	{ }
	virtual ~NpcImp() { }

	virtual bool OnInit()                          { return true; }
	virtual bool OnCanStroll()                     { return false; }
	virtual void OnHeartbeat()                     { }
	virtual int  OnMessageHandler(const MSG& msg)  { return -1; }
    virtual bool OnMapTeleport()                   { return false; }
    virtual bool OnMonsterMove(A2DVECTOR pos, float speed) { return false; }
    virtual void OnSetMonsterSpeed(float speed)    { }
    virtual void OnReachDestination(const A2DVECTOR& pos)  { }
    virtual bool OnPauseAICore()                   { return false; }
    virtual void OnHasInsertToWorld()              { }
    virtual bool OnCanCombat() const               { return false; }

	virtual int   GetRebornInterval()              { return kNpcDefaultRebornInterval; }
	virtual void  TriggerCombat(const XID& target) { ASSERT(false); }
	virtual float GetChaseSpeed() const            { return kNpcDefaultFollowSpeed; }

	virtual int   GetFaction()                     { return faction_; }
	virtual int   GetEnemyFaction()                { return enemy_faction_; }
    virtual void  SetBirthPlace(const A2DVECTOR& pos) { }


protected:
	Npc&    npc_;
	int     faction_;
	int     enemy_faction_;
};


#include "npc-inl.h"

} // namespace gamed

#endif // GAMED_GS_OBJ_NPC_H_
