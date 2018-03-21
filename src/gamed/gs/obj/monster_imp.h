#ifndef GAMED_GS_OBJ_MONSTER_IMP_H_
#define GAMED_GS_OBJ_MONSTER_IMP_H_

#include "shared/base/callback_bind.h"

#include "npc.h"
#include "npc_sender.h"


namespace mapDataSvr
{
	class SpotMonster;
	class AreaMonster;
}


namespace gamed {

struct MonsterElemInfo
{
	A2DVECTOR birth_place;          // 出生点
	uint8_t   birth_dir;            // 出生朝向
	int32_t   monster_group_id;     // 怪物组id
	int32_t   battle_scene_id;      // 战斗场景id
	uint8_t   ap_mode;              // 主动怪0，被动怪1
	bool      is_wandered;          // 是否随机走动
	uint8_t   combat_rule;          // 战斗规则
	int32_t   refresh_interval;     // 刷新间隔
	int32_t   patrol_path_id;       // 巡逻路径
	uint8_t   patrol_mode;          // 巡逻方式
	float     aggro_view_radius;    // 仇恨视野半径
	uint16_t  aggro_time;           // 仇恨时间，超过该时间则解除仇恨
	float     max_chase_distance;   // 最大追击距离，离开始追击点的直线距离
	float     chase_speed;          // 追击速度
};


/**
 * @brief MonsterImp
 *    （1）不要直接访问Npc的成员变量（虽然是friend class）
 *         可以通过调用Npc的public及protected函数来实现
 */
class MonsterImp : public NpcImp
{
	static const int kWaitingCombatTimeoutSecs  = 5;
	static const int kMonsterModelMaxRetainSecs = 2*3600; // 最长两个小时
public:
	MonsterImp(Npc& npc);
	virtual ~MonsterImp();

	virtual bool OnInit();
	virtual bool OnCanStroll();
	virtual void OnHeartbeat();
	virtual int  OnMessageHandler(const MSG& msg);
    virtual bool OnMapTeleport();
    virtual bool OnMonsterMove(A2DVECTOR pos, float speed);
    virtual void OnSetMonsterSpeed(float speed);
    virtual void OnReachDestination(const A2DVECTOR& pos);
    virtual bool OnPauseAICore();
    virtual bool OnCanCombat() const;
	
	virtual int   GetRebornInterval();
	virtual void  TriggerCombat(const XID& target);
	virtual float GetChaseSpeed() const;
    virtual void  SetBirthPlace(const A2DVECTOR& pos);


protected:
	virtual void HandlePlayerTriggerCombat(RoleID playerid);
	virtual void HandleCombatStart(bool is_success, XID attacker, bool is_player_reply = false);
	virtual void HandleCombatEnd(const msg_combat_end& msg_param);
    virtual void HandlePTriggerCombatSuccess();
	virtual bool CheckStateToCombat();

	inline npcdef::MonsterStates get_state() const;

	
private:
	typedef shared::bind::Callback<bool (const XID&, const A2DVECTOR&, int)> SearchCallback;

	bool InitRuntimeData();
	bool CheckCombatCondition(RoleID playerid);
	void FillElemInfo(const mapDataSvr::SpotMonster* pmonster);
	void FillElemInfo(const mapDataSvr::AreaMonster* pmonster);
	void ReplyCombatTrigger(RoleID playerid, bool is_trigger_success);
	void AggressiveSearch();
	bool SearchPlayer(const SearchCallback& callback);
	void DriveMachine();
	void TryGoHome();
    bool CheckRebornState() const;

	inline bool is_aggressive_mode(); // 主动怪、被动怪
    inline bool has_path_move() const;


private:
	MonsterElemInfo       elem_info_;
	npcdef::MonsterStates state_;
	bool                  aggressive_mode_;
	int                   waiting_combat_timer_;
	bool                  is_retain_model_;
	MSG_MonsterType       monster_type_;
    float                 path_move_speed_; // 小于0表示没有设置移动路径
    bool                  can_combat_;
};

///
/// inline func
///
inline bool MonsterImp::is_aggressive_mode()
{
	return aggressive_mode_;
}

inline npcdef::MonsterStates MonsterImp::get_state() const
{
	return state_;
}

inline bool MonsterImp::has_path_move() const
{
    return path_move_speed_ > 0;
}

} // namespace gamed

#endif // GAMED_GS_OBJ_MONSTER_IMP_H_
