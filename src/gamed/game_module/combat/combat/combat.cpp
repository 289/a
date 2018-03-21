#include <stdlib.h>
#include <algorithm>
#include "combat.h"
#include "combat_player.h"
#include "combat_npc.h"
#include "combat_def.h"
#include "combat_msg.h"
#include "combat_man.h"
#include "mob_spawner.h"
#include "mob_manager.h"
#include "combat_scene_ai.h"

#include "skill/include/buff.h"
#include "client_proto/G2C_proto.h"

namespace combat
{

/****************************Combat********************************/
/****************************Combat********************************/
/****************************Combat********************************/
/****************************Combat********************************/

///
/// static variable
///
CombatSenderCallBack    Combat::CombatSenderCB;
CombatPVEResultCallBack Combat::CombatPVEResultCB;
CombatPVPResultCallBack Combat::CombatPVPResultCB;
CombatStartCallBack     Combat::CombatStartCB;
CombatPVEEndCallBack    Combat::CombatPVEEndCB;
CombatPVPEndCallBack    Combat::CombatPVPEndCB;
WorldBossStatusCallBack Combat::WorldBossStatusCB;

void Combat::RemoveUnitFromList(CombatUnitVec& unit_list, bool clear)
{
    for (size_t i = 0; i < unit_list.size(); ++i)
    {
        CombatUnit* obj = unit_list[i];
        if (obj == NULL)
        {
            continue;
        }
        if (obj->IsPlayer())
        {
            CombatPlayer* player = dynamic_cast<CombatPlayer*>(obj);
            s_pCombatMan->RemovePlayerFromMan(player);
            s_pCombatMan->FreePlayer(player);
        }
        else if (obj->IsNPC())
        {
            CombatNpc* npc = dynamic_cast<CombatNpc*>(obj);
            s_pCombatMan->RemoveNPCFromMan(npc);
            s_pCombatMan->FreeNPC(npc);
        }
        obj = NULL;
    }
    if (clear)
    {
        unit_list.clear();
    }
}

Combat::Combat(char type): Object(type),
	  map_id_(0),
	  world_id_(0),
	  scene_id_(0),
	  scene_ai_(NULL),
	  atb_man_(this),
      combat_state_(this),
	  creator_roleid_(0),
	  taskid_(0),
	  challengeid_(0),
	  tick_counter_(0),
	  round_counter_(0),
	  is_boss_combat_(0),
	  attacker_win_(false),
	  attacker_sneaked_(false),
	  defender_sneaked_(false),
      force_combat_result_(0)
{
	attacker_list_.resize(MAX_COMBAT_UNIT_NUM);
	defender_list_.resize(MAX_COMBAT_UNIT_NUM);
    memset(last_action_unit_, 0, sizeof(last_action_unit_) * sizeof(UnitID));
    memset(last_action_tick_, 0, sizeof(last_action_tick_) * sizeof(int32_t));
    recycle_list_.clear();
}

Combat::~Combat()
{
    RemoveUnitFromList(attacker_list_, true);
    RemoveUnitFromList(defender_list_, true);
    RemoveUnitFromList(recycle_list_, true);
}

bool Combat::NeedReviseATB(CombatUnit* unit)
{
    int32_t index = unit->GetParty() - 1;
    assert(index >= 0 && index <= 1);
    int32_t delta = 0;
    delta = atb_man_.TotalTick() - last_action_tick_[index];
    if (delta < 2)
    {
        DeActivateATB(2 - delta);
        return true;
    }
    return false;
}

void Combat::SetAction(CombatUnit* unit, uint32_t cur_tick)
{
    int32_t index = unit->GetParty() - 1;
    assert(index >= 0 && index <= 1);
    last_action_unit_[index] = unit->GetID();
    last_action_tick_[index] = cur_tick;
}

bool Combat::Init(WorldID world_id, CombatSceneID combat_scene_id)
{
	map_id_ = world_id & 0xFFFFFFFF;
	world_id_ = world_id;
	scene_id_ = combat_scene_id;

	//初始化ATBMan
	atb_man_.Init();

	//创建场景AI
	/*if (GetCombatSceneEventID(scene_id_) > 0)
	{
		scene_ai_ = new CombatSceneAI(this);
		if (!scene_ai_ || !scene_ai_->Init())
		{
			return false;
		}
	}

    //初始化状态
    combat_state_.Init();*/

	return OnInit();
}

bool Combat::AddCombatUnit(CombatUnit* unit, RoleID role_to_join)
{
    if (!unit)
    {
        return false;
    }

	if (Find(unit->GetXID()))
	{
		assert(false);
		return false;
	}

    if (unit->IsPlayer())
    {
        if (role_to_join > 0)
        {
            CombatPlayer* player = QueryPlayer(role_to_join);
            if (!player)
            {
                return false;
            }

            int party = LocateParty(player->GetID());
            ASSERT(party != PARTY_INVALID);

            unit->SetParty(party);
            if (party == PARTY_ATTACKER)
            {
		        return InsertCombatUnit(attacker_list_, unit);
            }
            else if (party == PARTY_DEFENDER)
            {
		        return InsertCombatUnit(defender_list_, unit);
            }
        }
        else
        {

#define HAS_UNIT(list) \
            for (size_t i = 0; i < list.size(); ++ i) \
            { \
                if (list[i]) \
                { \
                    exist = true; \
                    break; \
                } \
            }

            bool exist = false;
            HAS_UNIT(attacker_list_);
            if (!exist)
            {
                unit->SetParty(PARTY_ATTACKER);
                return InsertCombatUnit(attacker_list_, unit);
            }

            exist = false;
            HAS_UNIT(defender_list_);
            if (!exist)
            {
                unit->SetParty(PARTY_DEFENDER);
                return InsertCombatUnit(defender_list_, unit);
            }
        }
#undef HAS_UNIT

    }
    else if (unit->IsTeamNpc())
	{
        unit->SetParty(PARTY_ATTACKER);
		return InsertCombatUnit(attacker_list_, unit);
	}
	else if (unit->IsMob() || unit->IsBoss())
	{
        unit->SetParty(PARTY_DEFENDER);
		return InsertCombatUnit(defender_list_, unit);
    }

    assert(false);
    return false;
}

bool Combat::RmvCombatUnit(CombatUnit* unit)
{
    if (!unit)
    {
        return false;
    }

	if (!Find(unit->GetXID()))
	{
		assert(false);
		return false;
	}

	if (unit->IsPlayer() || unit->IsTeamNpc())
	{
		return DeleteCombatUnit(attacker_list_, unit);
	}
	else if (unit->IsMob() || unit->IsBoss())
	{
		return DeleteCombatUnit(defender_list_, unit);
	}
	else
	{
		assert(false);
	}

    return false;
}

bool Combat::AddTeamNpc(TemplID npc_id, int npc_pos, CombatPlayer* player)
{
	NpcInfo npc_info;
	npc_info.id = npc_id;
	npc_info.pos = npc_pos;
	npc_info.lvl = s_mob_man.GetMobLevel(npc_id);
	CombatNpc* npc = CreateCombatNpc(npc_info, CombatNpc::TYPE_NPC_TEAMNPC);
	if (!npc)
	{
		return false;
	}

	npc->SetOwner(player);
	if (!AddCombatUnit(npc))
	{
        recycle_list_.push_back(npc);
		//s_pCombatMan->RemoveNPCFromMan(npc);
		//s_pCombatMan->FreeNPC(npc);
		npc->Unlock();
		return false;
	}

	npc->Unlock();
	return true;
}

bool Combat::StartCombat()
{
	//检查偷袭状态
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit) continue;
		if (attacker_sneaked_)
		{
			unit->SetSneaked();
		}
	}
	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (!unit) continue;
		if (defender_sneaked_)
		{
			unit->SetSneaked();
		}
	}

	//创建场景AI
	if (GetCombatSceneEventID(scene_id_) > 0)
	{
		scene_ai_ = new CombatSceneAI(this);
		if (!scene_ai_ || !scene_ai_->Init())
		{
		    __PRINTF("初始化场景脚本失败 scene_id=%d", scene_id_);
			return false;
		}
	}

    //初始化状态
    combat_state_.Init();

    OnStartCombat();

	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit) continue;
		unit->CombatStart();
	}

	Trace();
    return true;
}

void Combat::CombatEnd()
{
#define NOTIFY_DYING_PLAYER_DEAD(list) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
			if (unit && unit->IsPlayer() && unit->IsDying()) \
			{ \
                unit->OnDeath(); \
			} \
		} \
	}

	//濒死玩家-->死亡
	NOTIFY_DYING_PLAYER_DEAD(attacker_list_);
	NOTIFY_DYING_PLAYER_DEAD(defender_list_);
#undef NOTIFY_DYING_PLAYER_DEAD

    //更新战场状态
    combat_state_.Update(XEVENT_CLOSE, COMBAT_END_LATENCY_TIME / MSEC_PER_TICK);

	//执行战斗结算
	OnCombatEnd();
}

void Combat::Suspend(int msec)
{
    if (CanSuspendCombat())
    {
        if (combat_state_.IsStateOpenReady())
        {
            __PRINTF("----------------------脚本暂停战斗(%d), 持续%dmsec!!!", xid_.id, msec);
        }
        else if (combat_state_.IsStateRunning())
        {
            atb_man_.DeActivate();
            __PRINTF("----------------------脚本暂停战斗(%d), 持续%dmsec!!!", xid_.id, msec);
        }
        else if (combat_state_.IsStateSuspend())
        {
            __PRINTF("----------------------脚本二次暂停战斗(%d), 持续%dmsec!!!", xid_.id, msec);
        }
        else
        {
            assert(false);
        }

        int tick = msec / MSEC_PER_TICK;
        combat_state_.Update(XEVENT_SUSPEND, tick);
    }
}

void Combat::WaitSelectSkill(int skill_index, int msec, int talk_id)
{
    if (msec != 0)
    {
        if (CanWaitSelectSkill())
        {
            if (combat_state_.IsStateRunning())
            {
                atb_man_.DeActivate();
                __PRINTF("----------------------脚本等待选择技能(%d), 持续%dmsec!!!", xid_.id, msec);
            }
            else if (combat_state_.IsStateWaitSelectSkill())
            {
                __PRINTF("----------------------脚本二次暂停战斗(%d), 持续%dmsec!!!", xid_.id, msec);
            }
            else
            {
                assert(false);
            }

            // 通知客户端选择指定技能
            G2C::CombatWaitSelectSkill packet;
            packet.skill_index = skill_index;
            if (msec >= 5000)
            {
                msec -= 5000;
            }
            packet.wait_msec = msec;
            packet.talk_id = talk_id;
            BroadCastCMD(packet);
            int tick = msec / MSEC_PER_TICK;
            combat_state_.Update(XEVENT_WAIT_SELECT_SKILL, tick);
        }
    }
    else
    {
        combat_state_.Update(XEVENT_RESUME);
        scene_ai_->Resume();
    }
}

void Combat::WaitSelectPet(int pet_index, int msec, int talk_id)
{
    if (msec != 0)
    {
        if (CanWaitSelectPet())
        {
            if (combat_state_.IsStateRunning())
            {
                atb_man_.DeActivate();
                __PRINTF("----------------------脚本等待选择宠物(%d), 持续%dmsec!!!", xid_.id, msec);
            }
            else if (combat_state_.IsStateWaitSelectPet())
            {
                __PRINTF("----------------------脚本二次暂停战斗(%d), 持续%dmsec!!!", xid_.id, msec);
            }
            else
            {
                assert(false);
            }

            // 通知客户端选择指定技能
            G2C::CombatWaitSelectPet packet;
            packet.pet_index = pet_index;
            if (msec >= 5000)
            {
                msec -= 5000;
            }
            packet.wait_msec = msec;
            packet.talk_id = talk_id;
            BroadCastCMD(packet);
            int tick = msec / MSEC_PER_TICK;
            combat_state_.Update(XEVENT_WAIT_SELECT_PET, tick);
        }
    }
    else
    {
        combat_state_.Update(XEVENT_RESUME);
        scene_ai_->Resume();
    }
}

bool Combat::Terminate()
{
	//强制结束战斗
    combat_state_.Update(XEVENT_CLOSING);
	return true;
}

void Combat::Clear()
{
	ClearAllUnits();

	if (scene_ai_)
	{
		delete scene_ai_;
	}

	Object::Clear();

	map_id_   = 0;
	world_id_ = 0;
	scene_id_ = 0;
	scene_ai_ = NULL;
	atb_man_.Clear();
	pet_set_.clear();
    cmd_vec_.clear();
	combat_buffer_.Clear();
    combat_state_.Clear();
	attacker_win_ = false;
	creator_roleid_ = 0;
	taskid_ = 0;
	challengeid_ = 0;
	tick_counter_ = 0;
	round_counter_ = 0;
	is_boss_combat_ = false;
	attacker_sneaked_ = false;
	defender_sneaked_ = false;
    force_combat_result_ = 0;
}

bool Combat::TestCloseCombat()
{
	///
	/// 测试战斗是否可以执行结束操作？
    /// 注意：调用本函数的时候，说明战斗已经满足结束的条件了，只不过战斗对象的动作还没有播完而已。
	///       如果战场是正常结束，则一方全部处于死亡状态则战斗正式结束了
    ///       如果战场是异常结束，则全场没有在行动或被攻击则战斗结束了
	///

#define TEST_CLOSE_COMBAT(list) \
    { \
	    for (size_t i = 0; i < list.size(); ++ i) \
	    { \
		   	CombatUnit* unit = list[i]; \
		   	if (!unit) continue; \
            if (unit->IsAction() || unit->IsAttacked() || unit->IsZombie()) \
            { \
                return false; \
            } \
	    } \
    }

	TEST_CLOSE_COMBAT(attacker_list_);
	TEST_CLOSE_COMBAT(defender_list_);
#undef TEST_CLOSE_COMBAT

	return true;
}

bool Combat::TestCombatEnd()
{
	///
	/// 测试战斗是否满足结束的条件
	/// 注意：这里是只是测试，但是还不能立马关闭战场，
	///       关闭战场需要等到玩家全部回位后才可以执行
	///

    int32_t end = IsCombatEnd();

	if (end & 0x02)
	{
		//攻方失败
        UnRegisterAllATB();
		attacker_win_ = false;
        combat_state_.Update(XEVENT_CLOSING);
		return true;
	}

	if (end & 0x01)
	{
		//攻方取胜
        UnRegisterAllATB();
		attacker_win_ = true;
        combat_state_.Update(XEVENT_CLOSING);
		return true;
	}

	return false;
}

int32_t Combat::IsCombatEnd()
{
    if (force_combat_result_ != 0)
    {
        return force_combat_result_;
    }

	bool empty1 = true;
	bool empty2 = true;

	/*其中一方全部处于死亡/濒死/僵死状态, 则战斗结束*/
#define TEST_ALL_UNIT_DEAD(list,empty) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
			if (!unit) continue; \
			if (unit->IsAlive()) \
			{ \
				empty = false; \
				break; \
			} \
		} \
	}

	TEST_ALL_UNIT_DEAD(attacker_list_, empty1);
	TEST_ALL_UNIT_DEAD(defender_list_, empty2);
#undef TEST_ALL_UNIT_DEAD

    int32_t end = 0;
    if (empty1)
    {
        end |= 0x02;
    }
    if (empty2)
    {
        end |= 0x01;
    }
    return end;
}

void Combat::AssertZombie()
{
/*#define ASSERT_ZOMBIE(list) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
			if (unit && unit->IsZombie()) \
			{ \
				assert(unit->GetStateTimeout() != -1); \
			} \
		} \
	}

	ASSERT_ZOMBIE(attacker_list_);
	ASSERT_ZOMBIE(defender_list_);
#undef ASSERT_ZOMBIE*/
    for (size_t i = 0; i < attacker_list_.size(); ++i)
    {
        CombatUnit* unit = attacker_list_[i];
        if (unit && unit->IsZombie())
        {
            assert(unit->GetStateTimeout() != -1);
        }
    }
    for (size_t i = 0; i < defender_list_.size(); ++i)
    {
        CombatUnit* unit = defender_list_[i];
        if (unit && unit->IsZombie())
        {
            assert(unit->GetStateTimeout() != -1);
        }
    }
}

bool Combat::IsStateOpen() const
{
    return combat_state_.IsStateOpenReady() ||
           combat_state_.IsStateRunning() ||
           combat_state_.IsStateStopped() ||
           combat_state_.IsStateSuspend() ||
           combat_state_.IsStateWaitSelectSkill() ||
           combat_state_.IsStateWaitSelectPet();
}

bool Combat::IsStateClose() const
{
    return combat_state_.IsStateCloseWait() ||
           combat_state_.IsStateLastWait();
}

bool Combat::IsStateRunning() const
{
    return combat_state_.IsStateRunning();
}

bool Combat::IsStateSuspend() const
{
    return combat_state_.IsStateSuspend();
}

bool Combat::IsStateWaitSelectSkill() const
{
    return combat_state_.IsStateWaitSelectSkill();
}

bool Combat::IsStateWaitSelectPet() const
{
    return combat_state_.IsStateWaitSelectPet();
}

CombatUnit* Combat::Find(UnitID unit_id)
{
	CombatUnitVec::iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != attacker_list_.end())
		return *it;

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != defender_list_.end())
		return *it;

	return NULL;
}

const CombatUnit* Combat::Find(UnitID unit_id) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != attacker_list_.end())
		return *it;

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != defender_list_.end())
		return *it;

	return NULL;
}

CombatUnit* Combat::Find(const XID& xid)
{
	return Find(xid.id);
}

const CombatUnit* Combat::Find(const XID& xid) const
{
	return Find(xid.id);
}

struct PlayerFinder
{
	RoleID roleid;
	PlayerFinder(RoleID id): roleid(id) {}
	bool operator()(const CombatUnit* unit) const
	{
		if (!unit || !unit->IsPlayer()) return false;
		const CombatPlayer* player = dynamic_cast<const CombatPlayer*>(unit);
		return roleid == player->GetRoleID();
	}
};

CombatPlayer* Combat::QueryPlayer(RoleID roleid)
{
	const CombatUnitVec& list1 = attacker_list_;
	CombatUnitVec::const_iterator it = std::find_if(list1.begin(), list1.end(), PlayerFinder(roleid));
	if (it != list1.end())
	{
		return dynamic_cast<CombatPlayer*>(*it);
	}

	const CombatUnitVec& list2 = defender_list_;
	it = std::find_if(list2.begin(), list2.end(), PlayerFinder(roleid));
	return it != list2.end() ? dynamic_cast<CombatPlayer*>(*it) : NULL;
}

int Combat::LocateParty(UnitID unit_id) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != attacker_list_.end())
		return PARTY_ATTACKER;

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != defender_list_.end())
		return PARTY_DEFENDER;

    return PARTY_INVALID;
}

CombatUnit* Combat::FindKiller(UnitID unit_id)
{
    CombatUnit* unit = Find(unit_id);
    if (unit == NULL)
    {
        unit = QueryCombatPet(unit_id);
    }
    if (unit == NULL)
    {
        unit = QueryGolem(unit_id);
    }
    return unit;
}

UnitID Combat::GetOwnerID(UnitID unit_id)
{
    CombatUnit* unit = QueryGolem(unit_id);
    if (unit == NULL)
    {
        return unit_id;
    }
	CombatNpc* npc = dynamic_cast<CombatNpc*>(unit);
    assert(npc->IsGolem());
    return npc->GetOwner()->GetID();
}

void Combat::GetTeamMate(UnitID unit_id, CombatUnitVec& list) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != attacker_list_.end())
	{
		list = attacker_list_;
		return;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != defender_list_.end())
	{
		list = defender_list_;
	}
}

void Combat::GetEnemy(UnitID unit_id, CombatUnitVec& list) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != attacker_list_.end())
	{
		list = defender_list_;
		return;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(unit_id));
	if (it != defender_list_.end())
	{
		list = attacker_list_;
	}
}

void Combat::GetTeamMate(const XID& id, CombatUnitVec& list) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(id.id));
	if (it != attacker_list_.end())
	{
		list = attacker_list_;
		return;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(id.id));
	if (it != defender_list_.end())
	{
		list = defender_list_;
	}
}

void Combat::GetEnemy(const XID& id, CombatUnitVec& list) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(id.id));
	if (it != attacker_list_.end())
	{
		list = defender_list_;
		return;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(id.id));
	if (it != defender_list_.end())
	{
		list = attacker_list_;
	}
}

void Combat::GetGolem(CombatUnitVec& list) const
{
    GolemSet::const_iterator it = golem_set_.begin();
    for (; it != golem_set_.end(); ++it)
    {
        list.push_back(*it);
    }
}

int Combat::GetEnemiesAlive(const XID& xid) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(xid.id));
	if (it != attacker_list_.end())
	{
		int ret = 0;
		for (size_t i = 0; i < defender_list_.size(); ++ i)
		{
			CombatUnit* unit = defender_list_[i];
			if (!unit) continue;
			if (unit->IsAlive())
				++ ret;
		}
		return ret;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(xid.id));
	if (it != defender_list_.end())
	{
		int ret = 0;
		for (size_t i = 0; i <defender_list_.size(); ++ i)
		{
			CombatUnit* unit = attacker_list_[i];
			if (!unit) continue;
			if (unit->IsAlive())
				++ ret;
		}
		return ret;
	}

	assert(false);
	return -1;
}

int Combat::GetTeammatesAlive(const XID& xid) const
{
	CombatUnitVec::const_iterator it = std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(xid.id));
	if (it != attacker_list_.end())
	{
		int ret = 0;
		for (size_t i = 0; i < attacker_list_.size(); ++ i)
		{
			CombatUnit* unit = attacker_list_[i];
			if (!unit) continue;
			if (unit->IsAlive())
				++ ret;
		}
		return ret;
	}

	it = std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(xid.id));
	if (it != defender_list_.end())
	{
		int ret = 0;
		for (size_t i = 0; i <defender_list_.size(); ++ i)
		{
			CombatUnit* unit = defender_list_[i];
			if (!unit) continue;
			if (unit->IsAlive())
				++ ret;
		}
		return ret;
	}

	assert(false);
	return -1;
}

bool Combat::IsTeammate(UnitID uid1, UnitID uid2) const
{
	if (std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(uid1)) != attacker_list_.end())
	{
		for (size_t i = 0; i < attacker_list_.size(); ++ i)
		{
			CombatUnit* unit = attacker_list_[i];
			if (unit && unit->GetID() == uid2)
				return true;
		}
	}

	if (std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(uid1)) != defender_list_.end())
	{
		for (size_t i = 0; i < defender_list_.size(); ++ i)
		{
			CombatUnit* unit = defender_list_[i];
			if (unit && unit->GetID() == uid2)
				return true;
		}
	}
	return false;
}

bool Combat::HasSomeoneAction() const
{
#define TEST_UNIT_ACTION(list) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
            if (!unit) continue; \
			if (unit->IsAction() || unit->IsAttacked()) \
			{ \
				return true; \
			} \
		} \
	}

	TEST_UNIT_ACTION(attacker_list_);
	TEST_UNIT_ACTION(defender_list_);
#undef TEST_UNIT_ACTION

	return false;
}

void Combat::GetMobList(std::vector<TemplID>& list) const
{
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (unit && unit->IsMob())
        {
            list.push_back(((CombatNpc*)(unit))->GetNpcID());
        }
    }

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (unit && unit->IsMob())
        {
            list.push_back(((CombatNpc*)(unit))->GetNpcID());
        }
    }
}

void Combat::GetRoleClsList(std::vector<int>& list) const
{
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (unit && unit->IsPlayer())
        {
            list.push_back(((CombatPlayer*)(unit))->GetCls());
        }
	}

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (unit && unit->IsPlayer())
        {
            list.push_back(((CombatPlayer*)(unit))->GetCls());
        }
	}
}

bool Combat::IsEnemy(UnitID uid1, UnitID uid2) const
{
	if (std::find_if(attacker_list_.begin(), attacker_list_.end(), unit_finder<CombatUnit>(uid1)) != attacker_list_.end())
	{
		for (size_t i = 0; i < attacker_list_.size(); ++ i)
		{
			CombatUnit* unit = attacker_list_[i];
			if (unit && unit->GetID() == uid2)
				return false;
		}
	}

	if (std::find_if(defender_list_.begin(), defender_list_.end(), unit_finder<CombatUnit>(uid1)) != defender_list_.end())
	{
		for (size_t i = 0; i < defender_list_.size(); ++ i)
		{
			CombatUnit* unit = defender_list_[i];
			if (unit && unit->GetID() == uid2)
				return false;
		}
	}
	return true;
}

void Combat::SendMSG(const MSG& msg, size_t latency)
{
	s_pCombatMan->SendMSG(msg, latency);
}

void Combat::SendMSG(const ObjectVec& list, const MSG& msg, size_t latency)
{
	s_pCombatMan->SendMSG(list, msg, latency);
}

void Combat::SendMSG(const XID* first, const XID* last, const MSG& msg, size_t latency)
{
	s_pCombatMan->SendMSG(first, last, msg, latency);
}

void Combat::MessageHandler(const MSG& msg)
{
	assert(msg.target.IsCombat());
	assert(msg.target.id == xid_.id);

    switch (msg.message)
    {
        case COMBAT_MSG_SOMEONE_DEAD:
            {
                MsgHandler_SomeOneDead(msg);
            }
            break;
        case COMBAT_MSG_COMBAT_END:
            {
                MsgHandler_CombatEnd(msg);
            }
            break;
        default:
            {
                if (!OnMessageHandler(msg))
                {
                    assert(false && "未知消息类型");
                }
            }
            break;
    }
}

void Combat::BroadcastSkillResult(CombatUnit* pattacker, const SkillDamageVec& damages)
{
    if (damages.empty())
    {
        return;
    }

	G2C::CombatSkillResult packet;
	packet.combat_id = xid_.id;
	packet.attacker = pattacker->GetID();
    packet.pet_combat_pos = -1;
	packet.skill_result_list.resize(damages.size());
	for (size_t i = 0; i < damages.size(); ++ i)
	{
		const skill::SkillDamage& damage = damages[i];
		G2C::skill_result& result = packet.skill_result_list[i];

		result.skill_id = damage.skillid;
		result.attack_pos = damage.cast_pos;
		result.frame_list.resize(damage.frames.size());
		for (size_t j = 0; j < damage.frames.size(); ++ j)
		{
			result.frame_list[j].target_list.resize(damage.frames[j].players.size());
			result.frame_list[j].redir_target_list.resize(damage.frames[j].redir_players.size());

			for (size_t k = 0; k < damage.frames[j].players.size(); ++ k)
			{
				const skill::PlayerDamage& dmg = damage.frames[j].players[k];

				G2C::target_entry& target = result.frame_list[j].target_list[k];
				target.target = dmg.defender;
				target.effect_list.resize(dmg.dmgs.size());

				for (size_t m = 0; m < dmg.dmgs.size(); ++ m)
				{
					const skill::EffectDamage& info = dmg.dmgs[m];
					target.effect_list[m].effect_id = info.effectid;
					target.effect_list[m].status = info.status & skill::EFFECT_CRIT ? 1:0;
					for (size_t n = 0; n < info.dmgs.size(); ++ n)
					{
						G2C::prop_entry prop;
						prop.type  = info.dmgs[n].type;
						prop.index = info.dmgs[n].prop;
						prop.value = info.dmgs[n].value;
						target.effect_list[m].props.push_back(prop);
					}
				}
			}

			for (size_t k = 0; k < damage.frames[j].redir_players.size(); ++ k)
			{
				const skill::PlayerDamage& dmg = damage.frames[j].redir_players[k];

				G2C::target_entry& target = result.frame_list[j].redir_target_list[k];
				target.target = dmg.defender;
				target.effect_list.resize(dmg.dmgs.size());

				for (size_t m = 0; m < dmg.dmgs.size(); ++ m)
				{
					const skill::EffectDamage& info = dmg.dmgs[m];
					target.effect_list[m].effect_id = info.effectid;
					target.effect_list[m].status = info.status & skill::EFFECT_CRIT ? 1:0;
					for (size_t n = 0; n < info.dmgs.size(); ++ n)
					{
						G2C::prop_entry prop;
						prop.type  = info.dmgs[n].type;
						prop.index = info.dmgs[n].prop;
						prop.value = info.dmgs[n].value;
						target.effect_list[m].props.push_back(prop);
					}
				}
			}
		}
	}

	//确定攻击者类型
	if (pattacker->IsPlayer())
	{
		packet.attacker_type = G2C::CombatSkillResult::ATTACKER_TYPE_PLAYER;
	}
	else if (pattacker->IsNPC())
	{
		CombatNpc* npc = dynamic_cast<CombatNpc*>(pattacker);
		if (npc->IsPet())
		{
			packet.attacker = npc->GetOwner()->GetID();
			packet.attacker_type = G2C::CombatSkillResult::ATTACKER_TYPE_PET;
            packet.pet_combat_pos = npc->GetPetCombatPos();
		}
		else if (npc->IsGolem())
		{
			packet.attacker = npc->GetOwner()->GetID();
			packet.attacker_type = G2C::CombatSkillResult::ATTACKER_TYPE_GOLEM;
		}
		else if (npc->IsMob() || npc->IsTeamNpc())
		{
			packet.attacker_type = G2C::CombatSkillResult::ATTACKER_TYPE_MOB;
		}
		else if (npc->IsBoss())
		{
			packet.attacker_type = G2C::CombatSkillResult::ATTACKER_TYPE_BOSS;
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
	}

	BroadCastCMD(packet);
}

void Combat::BroadcastBuffResult(const skill::BuffDamageVec& damages)
{
	if (damages.empty())
	{
		return;
	}

	G2C::CombatBuffResult packet;
	packet.combat_id = xid_.id;
	packet.buff_list.resize(damages.size());
	for (size_t i = 0; i < damages.size(); ++ i)
	{
		const skill::BuffDamage& damage = damages[i];

		G2C::buff_entry& buff = packet.buff_list[i];
		buff.attacker  = damage.attacker;
		buff.target    = damage.defender;
		buff.effect_id = damage.effectid;
		buff.buff_seq  = damage.buff_sn;

		for (size_t j = 0; j < damage.dmgs.size(); ++ j)
		{
			G2C::prop_entry prop;
			prop.type  = damage.dmgs[j].type;
			prop.index = damage.dmgs[j].prop;
			prop.value = damage.dmgs[j].value;
			buff.props.push_back(prop);
		}
	}

	BroadCastCMD(packet);
}

void Combat::RegisterATB(CombatUnit* unit)
{
	if (CanRegisterATB())
	{
		atb_man_.RegisterATB(unit);
	}
}

void Combat::RegisterInstantATB(CombatUnit* unit)
{
	if (CanRegisterATB())
	{
		atb_man_.RegisterInstantATB(unit);
	}
}

void Combat::UnRegisterATB(UnitID unit_id)
{
    if (CanUnRegisterATB())
    {
	    atb_man_.UnRegisterATB(unit_id);
    }
}

void Combat::UnRegisterAllATB()
{
	atb_man_.UnRegisterAllATB();
}

void Combat::ResetATB(CombatUnit* unit)
{
	if (CanRegisterATB())
    {
	    atb_man_.ResetATB(unit);
    }
}

void Combat::RegisterAllATB()
{
	//注册全部战斗对象的ATB
#define REGISTER_MULTI_ATB(list) \
	for (size_t i = 0; i < list.size(); ++ i) \
	{ \
		CombatUnit* unit = list[i]; \
		if (unit && unit->IsAlive()) \
		{ \
			unit->RegisterATB(); \
		} \
	}

	REGISTER_MULTI_ATB(attacker_list_);
	REGISTER_MULTI_ATB(defender_list_);
#undef REGISTER_MULTI_ATB
}

void Combat::ActivateATB()
{
	atb_man_.Activate();
}

void Combat::DeActivateATB(int tick)
{
    if (CanStopCombat())
    {
        //__PRINTF("----------------------战斗对象暂停战斗(%d), 持续%dmsec!!!", xid_.id, tick * MSEC_PER_TICK);

        atb_man_.DeActivate();

        combat_state_.Update(XEVENT_STOP, tick);
    }
}

void Combat::OnATBChange(CombatUnit* unit)
{
	atb_man_.OnATBChange(unit);
}

void Combat::HeartBeat()
{
	if (++ tick_counter_ == 1)
	{
		//首次心跳,通知gs战场创建完成
		//CombatStartCB(creator_roleid_, xid_.id);
	}

	//对战斗对象做心跳
	HeartBeatAllUnits();

	//对ATB管理器做心跳
	atb_man_.HeartBeat();

    //对状态做心跳
    combat_state_.HeartBeat();

    if (combat_state_.IsStateRunning())
    {
        HandleDelayCMD();
    }

	//测试发送Buff数据
	BroadcastBuffData();
}

void Combat::AssertLocked()
{
	mutex_.AssertLocked();
}

void Combat::OnRoundStart(UnitID unit_id)
{
	++ round_counter_;
	if (scene_ai_)
	{
		scene_ai_->OnRoundStart(unit_id);
	}
}

void Combat::OnRoundEnd(UnitID unit_id)
{
/*#define GEN_PET_POWER(list) \
    { \
        for (size_t i = 0; i < list.size(); ++ i) \
        { \
            CombatUnit* unit = list[i]; \
            if (!unit || unit->GetID() == unit_id || !unit->IsPlayer()) \
                continue; \
            CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit); \
            if (player->GetAttackPetID() != unit_id) \
            { \
                player->GenPetPower(); \
            } \
        } \
    }*/
#define GEN_PET_POWER(list) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
			if (!unit || !unit->IsPlayer()) \
				continue; \
			CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit); \
			if (player->GetAttackPetID() != unit_id && player->GetCurGolemID() != unit_id) \
			{ \
				player->GenPetPower(); \
			} \
		} \
	}

	GEN_PET_POWER(attacker_list_);
	GEN_PET_POWER(defender_list_);

#undef GEN_PET_POWER
    //if (scene_ai_)
    //{
        //scene_ai_->OnRoundEnd(unit_id);
    //}
}

void Combat::SetCombatCMDSenderCallBack(const CombatSenderCallBack& cb)
{
	CombatSenderCB = cb;
}

void Combat::SetCombatPVEResultCallBack(const CombatPVEResultCallBack& cb)
{
	CombatPVEResultCB = cb;
}

void Combat::SetCombatPVPResultCallBack(const CombatPVPResultCallBack& cb)
{
	CombatPVPResultCB = cb;
}

void Combat::SetCombatStartCallBack(const CombatStartCallBack& cb)
{
	CombatStartCB = cb;
}

void Combat::SetCombatPVEEndCallBack(const CombatPVEEndCallBack& cb)
{
	CombatPVEEndCB = cb;
}

void Combat::SetCombatPVPEndCallBack(const CombatPVPEndCallBack& cb)
{
	CombatPVPEndCB = cb;
}

void Combat::SetWorldBossStatusCallBack(const WorldBossStatusCallBack& cb)
{
	WorldBossStatusCB = cb;
}

bool Combat::HandleCMD(const PlayerCMD& cmd)
{
    if (!CanPlayerOperate())
        return false;

    if (combat_state_.IsStateCloseWait())
    {
        cmd_vec_.push_back(cmd);
        return true;
    }
    else
    {
        CombatPlayer* player = QueryPlayer(cmd.roleid);
        if (!player)
        {
            return false;
        }
        return player->HandleCMD(cmd);
    }
}

void Combat::ClearAllUnits()
{
    RemoveUnitFromList(attacker_list_, false);
    RemoveUnitFromList(defender_list_, false);
    RemoveUnitFromList(recycle_list_, true);
}

void Combat::HandleDelayCMD()
{
    for (size_t i = 0; i < cmd_vec_.size(); ++i)
    {
        HandleCMD(cmd_vec_[i]);
    }
    cmd_vec_.clear();
}

void Combat::HeartBeatAllUnits()
{
#define HEARTBEAT_UNIT_LIST(list) \
	{ \
		for (size_t i = 0; i < list.size(); ++ i) \
		{ \
			CombatUnit* unit = list[i]; \
			if (!unit) continue; \
			if (!unit->IsDead()) \
			{ \
				unit->HeartBeat(); \
			} \
		} \
	}

	//HEARTBEAT_UNIT_LIST(attacker_list_);
	{
		for (size_t i = 0; i < attacker_list_.size(); ++ i)
		{
			CombatUnit* unit = attacker_list_[i];
			if (!unit) continue;
			if (!unit->IsDead())
			{
				unit->HeartBeat();
			}
		}
	}
	HEARTBEAT_UNIT_LIST(defender_list_);
#undef HEARTBEAT_UNIT_LIST
}

void Combat::DoBroadCastCMD(CombatUnitVec& list, shared::net::ProtoPacket& packet, UnitID ex_id)
{
	for (size_t i = 0; i < list.size(); ++ i)
	{
		CombatUnit* unit = list[i];
		if (!unit || unit->GetID() == ex_id)
			continue;

		if (!unit->IsPlayer())
			continue;

		CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);
		if (player->IsOffline())
			continue;

		player->SendPacket(packet);
	}
}

CombatNpc* Combat::CreateCombatNpc(const NpcInfo& npc_info, char npc_type)
{
	CombatNpc* npc = s_pCombatMan->AllocNPC();
	if (!npc)
	{
		return NULL;
	}

	npc->SetNpcID(npc_info.id);
	npc->SetNpcType(npc_type);
	npc->Initialize();
	npc->SetPos(npc_info.pos);
	npc->SetLevel(npc_info.lvl);
	npc->SetCombat(this);

	//NPC对象放入对象池
	s_pCombatMan->InsertNPCToMan(npc);
	return npc;
}

bool Combat::InsertCombatUnit(CombatUnitVec& list, CombatUnit* unit)
{
	int pos = unit->GetPos();
	if (pos < MIN_COMBAT_POS || pos > MAX_COMBAT_POS)
	{
		//非法位置
		return false;
	}

	if (!list[pos])
	{
		//空位
		list[pos] = unit;
		return true;
	}

	//已占用,重新分配
	int i = list.size() - 1;
	for (; i >= 0; -- i)
	{
		if (list[i]) continue;
		list[i] = unit;
		list[i]->SetPos(i);
		break;
	}
	return i >= 0;
}

bool Combat::DeleteCombatUnit(CombatUnitVec& list, CombatUnit* unit)
{
    for (size_t i = 0; i < list.size(); ++ i)
    {
        if (list[i] == unit)
        {
            if (unit->IsMob() || unit->IsNPC())
            {
                list[i] = NULL;
                recycle_list_.push_back(unit);
                //unit->Clear();
				//s_pCombatMan->RemoveNPCFromMan(dynamic_cast<CombatNpc*>(unit));
				//s_pCombatMan->FreeNPC(dynamic_cast<CombatNpc*>(unit));
                return true;
            }
            else
            {
                //暂时只支持删除怪物
                assert(false);
                return false;
            }
        }
    }

    return false;
}

void Combat::BroadcastBuffData()
{
	if (combat_buffer_.IsEmpty())
		return;

	const CombatBuffer::BuffVec& listadd = combat_buffer_.GetBuffersAdd();
	const CombatBuffer::BuffVec& listdel = combat_buffer_.GetBuffersDel();

	G2C::CombatBuffData packet;
	for (size_t i = 0; i < listadd.size(); ++ i)
	{
		G2C::CombatBuffInfo buff;
		buff.buff_seq = listadd[i].buff_seq;
		buff.buff_id  = listadd[i].buff_id;
		buff.attacher = GetOwnerID(listadd[i].attacher);
		buff.target   = listadd[i].target;

		packet.buff_list_add.push_back(buff);
	}

	for (size_t i = 0; i < listdel.size(); ++ i)
	{
		G2C::CombatBuffInfo buff;
		buff.buff_seq = listdel[i].buff_seq;
		buff.buff_id  = listdel[i].buff_id;
		buff.attacher = GetOwnerID(listdel[i].attacher);
		buff.target   = listdel[i].target;

		packet.buff_list_del.push_back(buff);
	}

	BroadCastCMD(packet);
	combat_buffer_.Clear();
}

void Combat::BuildBuffSyncData(shared::net::ProtoPacket& packet)
{
    G2C::CombatBuffSync* pPacket = dynamic_cast<G2C::CombatBuffSync*>(&packet);
#define COLLECT_BUFF(__list) \
	for (size_t i = 0; i < __list.size(); ++ i) \
	{ \
		CombatUnit* unit = __list[i]; \
		if (!unit) continue; \
        skill::BuffWrapper* wrapper = unit->GetBuffWrapper(); \
        skill::BuffVec& buff_vec = wrapper->GetBuff(); \
        skill::BuffVec::const_iterator bit = buff_vec.begin(); \
        for (; bit != buff_vec.end(); ++bit) \
        { \
            G2C::CombatBuffInfo buff; \
            buff.buff_seq = bit->GetBuffSeq(); \
            buff.buff_id = bit->GetBuffId(); \
            buff.attacher = GetOwnerID(bit->GetAttacherId()); \
            buff.target = bit->GetTargetId(); \
            pPacket->buff_list.push_back(buff); \
        } \
	}

	COLLECT_BUFF(attacker_list_);
	COLLECT_BUFF(defender_list_);
#undef COLLECT_BUFF
}

CombatNpc* Combat::QueryCombatPet(UnitID pet_unit_id)
{
	CombatPetSet::const_iterator it = pet_set_.begin();
	for (; it != pet_set_.end(); ++ it)
	{
		if ((*it)->GetID() == pet_unit_id)
			return *it;
	}
	return NULL;
}

CombatNpc* Combat::QueryGolem(UnitID unit_id)
{
	GolemSet::const_iterator it = golem_set_.begin();
	for (; it != golem_set_.end(); ++ it)
	{
		if ((*it)->GetID() == unit_id)
			return *it;
	}
	return NULL;
}

void Combat::MsgHandler_SomeOneDead(const MSG& msg)
{
	//有战斗对象死亡了
    if (!combat_state_.IsStateCloseWait() && !combat_state_.IsStateLastWait())
    {
        UnitID deader = msg.source.id;
        UnitID killer = msg.param;

        CombatUnit* pDeader = Find(deader);
        CombatUnit* pKiller = FindKiller(killer);
        // pKiller可能找不到，比如被反弹伤害杀死的就是无来源的
        //assert(pKiller);

        if (pKiller)
        {
            //成功找到击杀者
            CombatPlayer* player = NULL;
            if (pKiller->IsPlayer())
            {
                player = dynamic_cast<CombatPlayer*>(pKiller);
            }
            else if (pKiller->IsPet() || pKiller->IsGolem() || pKiller->IsTeamNpc())
            {
                CombatUnit* owner = dynamic_cast<CombatNpc*>(pKiller)->GetOwner();
                player = dynamic_cast<CombatPlayer*>(owner);
            }

            //只处理击杀者是玩家的情况
            if (player)
            {
                if (pDeader->IsMob() || pDeader->IsBoss())
                {
                    player->KillMob(deader, dynamic_cast<CombatNpc*>(pDeader)->GetNpcID());
                }
                else if (pDeader->IsPlayer())
                {
                    player->KillPlayer(deader, dynamic_cast<CombatPlayer*>(pDeader)->GetRoleID());
                }
            }
        }

        OnSomeoneDead(deader, killer);
    }
}

void Combat::MsgHandler_CombatEnd(const MSG& msg)
{
    CombatEnd();
}

void Combat::Trace() const
{
	__PRINTF("----------------------创建战场成功----------------------");

#ifdef _COMBAT_DUMP
	__PRINTF("\n----------------------攻方阵营: ----------------------");
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		if (attacker_list_[i])
		{
			attacker_list_[i]->Trace();
		}
	}

	__PRINTF("\n----------------------守方阵营: ----------------------");
	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		if (defender_list_[i])
		{
			defender_list_[i]->Trace();
		}
	}

	__PRINTF("----------------------战场信息----------------------");
	__PRINTF("战场ID：%d", xid_.id);
	__PRINTF("地图ID：%d", map_id_);
	__PRINTF("大世界ID：%ld", world_id_);
	__PRINTF("战斗场景ID：%d", scene_id_);
	__PRINTF("战场类型: %s", GetType() == OBJ_TYPE_COMBAT_PVE ? "PVE" : "PVP");
#endif
}

/****************************CombatPVE***********************************************/
/****************************CombatPVE***********************************************/
/****************************CombatPVE***********************************************/
/****************************CombatPVE***********************************************/
CombatPVE::CombatPVE(): Combat(OBJ_TYPE_COMBAT_PVE),
	mob_group_id_(0),
	next_mob_group_id_(0),
	wave_counter_(0),
	wave_total_(0),
	world_boss_id_(0),
	world_monster_id_(0),
	init_player_count_(0)
{
}

CombatPVE::~CombatPVE()
{
}

bool CombatPVE::OnInit()
{
	//最少需要生成几只怪
	int32_t min_mob_count = init_player_count_;

	//设置战斗双方的偷袭状态
	//优先检查怪物是否被偷袭
	if (s_mob_spawner.TestSneakAttacked(mob_group_id_))
	{
		SetDefenderSneaked();
	}
	else if (s_mob_spawner.TestSneakAttack(mob_group_id_))
	{
		SetAttackerSneaked();
	}

	//生成怪物组
	std::vector<MobInfo> mob_list;
	if (!s_mob_spawner.GenerateMob(mob_group_id_, mob_list, min_mob_count))
	{
		return false;
	}

	int mobtype = 0;
	if (is_boss_combat_)
	{
		mobtype = CombatNpc::TYPE_NPC_BOSS;
	}
	else
	{
		mobtype = CombatNpc::TYPE_NPC_MOB;
	}

	//创建战斗怪
	for (size_t i = 0; i < mob_list.size(); ++ i)
	{
		CombatNpc* npc = CreateCombatNpc(mob_list[i], mobtype);
		if (!npc)
		{
			return false;
		}

		if (!AddCombatUnit(npc))
		{
            recycle_list_.push_back(npc);
			//s_pCombatMan->RemoveNPCFromMan(npc);
			//s_pCombatMan->FreeNPC(npc);
			npc->Unlock();
			return false;
		}
		npc->Unlock();
	}

	//总共有几波怪物
	TemplID __id = mob_group_id_;
	while (__id > 0)
	{
		++ wave_total_;
		__id = s_mob_spawner.GetNextMobGroupID(__id);
	};

	//设置下一波怪物的怪物组ID
	wave_counter_ = 1;
	next_mob_group_id_ = s_mob_spawner.GetNextMobGroupID(mob_group_id_);
	return true;
}

void CombatPVE::Clear()
{
	Combat::Clear();

	mob_group_id_      = 0;
	next_mob_group_id_ = 0;
	wave_counter_      = 0;
	wave_total_        = 0;
	world_boss_id_     = 0;
	world_monster_id_  = 0;
	init_player_count_ = 0;

	killed_mob_map_.clear();
}

void CombatPVE::OnStartCombat()
{
	//将战场信息同步给客户端
	G2C::CombatPVEStart packet;
    BuildCombatData(packet);
	BroadCastCMD(packet);
}

void CombatPVE::BuildCombatData(shared::net::ProtoPacket& packet)
{
	//将战场信息同步给客户端
	G2C::CombatPVEStart* pPacket = dynamic_cast<G2C::CombatPVEStart*>(&packet);
	pPacket->combat_id = xid_.id;
	pPacket->combat_scene_id = scene_id_;
	pPacket->part_sneak_attacked = G2C::CombatPVEStart::COMBAT_NO_SNEAK_ATTACKED;
	if (attacker_sneaked_)
		pPacket->part_sneak_attacked = G2C::CombatPVEStart::PLAYER_BE_SNEAK_ATTACKED;
	else if (defender_sneaked_)
		pPacket->part_sneak_attacked = G2C::CombatPVEStart::MOB_BE_SNEAK_ATTACKED;

	if (mob_group_id_ > 0)
		pPacket->wave_common_hint_id = s_mob_spawner.GetCombatCommonHintID(mob_group_id_);

	pPacket->wave_total = wave_total_;
	pPacket->new_combat = 1;

	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit)
			continue;

		if (!unit->IsPlayer() && unit->IsDead())
			continue;

		if (unit->IsPlayer())
		{
			G2C::CombatPlayerInfo pinfo;
			CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);
			player->SaveForClient(pinfo);
			pPacket->teammate_list.push_back(pinfo);
		}
		else if (unit->IsTeamNpc())
		{
			G2C::CombatMobInfo minfo;
			CombatNpc* npc = dynamic_cast<CombatNpc*>(unit);
			npc->SaveForClient(minfo);

			G2C::CombatTeamNpcInfo npc_info;
			npc_info.unit_id = minfo.unit_id;
			npc_info.npc_tid = minfo.mob_tid;
			npc_info.npc_pos = minfo.mob_pos;
			npc_info.sneak_attacked = minfo.sneak_attacked;
			pPacket->team_npc_list.push_back(npc_info);
		}
		else assert(false);
	}

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (!unit) continue;
		if (!unit->IsPlayer() && unit->IsDead()) continue;
		if (unit->IsMob() || unit->IsBoss())
		{
			G2C::CombatMobInfo minfo;
			CombatNpc* mob = dynamic_cast<CombatNpc*>(unit);
			mob->SaveForClient(minfo);
			pPacket->mob_list.push_back(minfo);
		}
		else assert(false);
	}
}

void CombatPVE::BroadCastCMD(shared::net::ProtoPacket& packet, UnitID ex_id)
{
	DoBroadCastCMD(attacker_list_, packet, ex_id);
}

CombatNpc* CombatPVE::GetBoss(int pos)
{
	if (pos >= MAX_COMBAT_UNIT_NUM)
		return NULL;
	return dynamic_cast<CombatNpc*>(defender_list_[pos]);
}

void CombatPVE::FindMob(UnitID mob_tid, std::vector<CombatUnit*>& list)
{
#define COLLECT_MOB(__list) \
	for (size_t i = 0; i < __list.size(); ++ i) \
	{ \
		CombatUnit* unit = __list[i]; \
		if (!unit || unit->IsPlayer()) continue; \
		CombatNpc* npc = dynamic_cast<CombatNpc*>(unit); \
		if (npc->GetNpcID() == mob_tid) \
		{ \
			list.push_back(unit); \
		} \
	}

	COLLECT_MOB(attacker_list_);
	COLLECT_MOB(defender_list_);
#undef COLLECT_MOB
}

bool CombatPVE::IsMonster(UnitID unit_id)
{
    for (int32_t i = 0; i < 2; ++i)
    {
        const CombatUnitVec& unit_list = i == 0 ? defender_list_ : attacker_list_;
        for (size_t j = 0; j < unit_list.size(); ++j)
        {
            const CombatUnit* unit = unit_list[j];
            if (unit && !unit->IsPlayer() && unit->GetID() == unit_id)
            {
                return true;
            }
        }
    }
	//for (size_t i = 0; i < defender_list_.size(); ++ i)
	//{
		//CombatUnit* unit = defender_list_[i];
		//if (unit && unit->GetID() == unit_id)
			//return true;
	//}

	return false;
}

void CombatPVE::OnMobKilled(TemplID mob_tid, int32_t money_drop, int32_t exp_drop, const std::vector<ItemEntry>& items_drop, UnitID killer)
{
	//统计杀死了哪些怪物
	killed_mob_map_[mob_tid] += 1;

	if (killer <= 0)
	{
		//击杀者未知，奖励丢弃
		return;
	}

	/*CombatUnit* pKiller = Find(killer);
	if (!pKiller)
	{
		//击杀者可能是宠物
		pKiller = QueryCombatPet(killer);
		assert(pKiller);
	}*/
    CombatUnit* pKiller = FindKiller(killer);
    assert(pKiller);

	CombatPlayer* player = NULL;
	if (pKiller->IsPlayer())
	{
		player = dynamic_cast<CombatPlayer*>(pKiller);
	}
	else if (pKiller->IsPet() || pKiller->IsGolem() || pKiller->IsTeamNpc())
	{
        CombatUnit* owner = dynamic_cast<CombatNpc*>(pKiller)->GetOwner();
        player = dynamic_cast<CombatPlayer*>(owner);
	}
	else
	{
		assert(false);
	}

	if (player)
	{
		player->GainCombatAward(items_drop, money_drop, exp_drop);
	}
	else
	{
		//非怪物死亡
		//组队NPC死亡
	}
}

void CombatPVE::OnCombatStart()
{
	//战斗正式开始了

	//注册ATB
	RegisterAllATB();

	//执行场景AI策略
	if (scene_ai_)
	{
		scene_ai_->OnCombatStart();
	}
}

void CombatPVE::DoCombatEnd()
{
	//计算战场中玩家个数
	std::vector<RoleID> playerlist;
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit || !unit->IsPlayer()) continue;

		CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);
		playerlist.push_back(player->GetRoleID());
	}

	//同步GS和客户端
	for (size_t k = 0; k < attacker_list_.size(); ++ k)
	{
		CombatUnit* unit = attacker_list_[k];
		if (!unit || !unit->IsPlayer()) continue;

		CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);

		int rst = attacker_win_ ? RESULT_WIN : RESULT_FAIL;
		const CombatAward& award = player->GetAward();

		//获得玩家击杀怪物
		std::vector<MobKilled> mob_killed_list;
		player->GetMobsKilled(mob_killed_list);

		CombatPVEResult result;
		result.result = rst;
        result.monster_group_id = mob_group_id_;
        result.challengeid = challengeid_;
        result.taskid = taskid_;
		result.hp = player->GetHP() >= 0 ? player->GetHP() : 0;
		result.exp = award.exp;
		result.money = award.money;
		result.pet_power = player->GetPetPower();
		result.mob_killed_vec = mob_killed_list;
		for (size_t i = 0; i < award.items.size(); ++ i)
			result.items.push_back(award.items[i]);
		for (size_t i = 0; i < award.lottery.size(); ++ i)
			result.lottery.push_back(award.lottery[i]);

		//同步战斗结果给GS
		Combat::CombatPVEResultCB(player->GetRoleID(), xid_.id, player->GetID(), result, player->GetMasterServerID());

		//通知玩家战斗结束
		Combat::CombatPVEEndCB(player->GetRoleID(), 0, xid_.id, attacker_win_, playerlist, mob_killed_list);
	}

	//统计共杀死多少怪物
	std::vector<MobKilled> mob_killed_list;
	KilledMobMap::const_iterator it = killed_mob_map_.begin();
	for (; it != killed_mob_map_.end(); ++ it)
	{
		MobKilled entry;
		entry.mob_tid = it->first;
		entry.mob_count = it->second;
		mob_killed_list.push_back(entry);
	}

	//通知怪物战斗结束
	Combat::CombatPVEEndCB(world_monster_id_, world_id_, xid_.id, !attacker_win_, playerlist, mob_killed_list);

	//通知世界BOSS有战斗结束
	if (is_boss_combat_)
	{
        MSG msg;
		int result = attacker_win_ ? RESULT_FAIL : RESULT_WIN;
        BuildMessage(msg, COMBAT_MSG_BOSS_COMBAT_END, MAKE_XID(world_boss_id_), xid_, result, NULL, 0);
        SendMSG(msg);
	}

    Clear();
    //s_pCombatMan->HeartBeat();

	s_pCombatMan->RemovePVECombatFromMan(this);
	s_pCombatMan->FreePVECombat(this);

	__PRINTF("----------------------销毁PVE战场(id=%d)----------------------", xid_.id);
}

bool CombatPVE::TryCloseCombat()
{
	if (attacker_win_ && next_mob_group_id_ > 0)
	{
		//继续战斗
		mob_group_id_ = next_mob_group_id_;
		next_mob_group_id_ = s_mob_spawner.GetNextMobGroupID(mob_group_id_);
		return false;
	}

	//战斗结束
	return true;
}

bool CombatPVE::CanJoinCombat() const
{
    if (next_mob_group_id_ > 0)
        return true;

    return IsStateOpen();
}

void CombatPVE::OnCombatEnd()
{
	///
	/// 战斗结束，执行战斗结算，包括2个方面:
	/// 1) 战斗物品、游戏币、经验值的发放;
	/// 2) 玩家击杀怪物个数的统计;
	///
	for (size_t k = 0; k < attacker_list_.size(); ++ k)
	{
		CombatUnit* unit = attacker_list_[k];
		if (!unit || !unit->IsPlayer())
            continue;

        CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);

		if (attacker_win_)
		{
			int32_t __drop_exp = 0;
			int32_t __drop_money = 0;
			std::vector<ItemEntry> __lottery_items;

			for (size_t i = 0; i < defender_list_.size(); ++ i)
			{
				if (!defender_list_[i])
                    continue;
	
				CombatNpc* mob = dynamic_cast<CombatNpc*>(defender_list_[i]);
				TemplID mob_tid = mob->GetNpcID();
	
				//获得经验和金钱
				__drop_exp += s_mob_man.GenerateDropExp(mob_tid);
				__drop_money += s_mob_man.GenerateDropMoney(mob_tid);

				//对金钱和经验校正
				__drop_exp   *= (1.0f + CombatAwardConfig::GetTeamAwardExpFactor(init_player_count_));
				__drop_money *= (1.0f + CombatAwardConfig::GetTeamAwardMoneyFactor(init_player_count_));
	
				//随机抽奖物品
				std::vector<ItemEntry> items;
				if (s_mob_man.GenerateLotteryItem(mob_tid, items))
				{
					__lottery_items.insert(__lottery_items.begin(), items.begin(), items.end());
				}

                //统计击杀了多少怪数
				player->KillMob(mob->GetID(), mob_tid);
			}
	
			//玩家获得奖励
			player->GainCombatExp(__drop_exp);
			player->GainCombatMoney(__drop_money);
			for (size_t j = 0; j < __lottery_items.size(); ++ j)
			{
				player->GainLotteryItem(__lottery_items[j].item_id, __lottery_items[j].item_count);
			}
		}
	}
}

void CombatPVE::OnSomeoneDead(UnitID deader, UnitID killer)
{
    if (IsMonster(deader))
    {
        CombatUnitVec list;
        GetTeamMate(deader, list);
        for (size_t i = 0; i < list.size(); ++ i)
        {
            CombatUnit* unit = list[i];
            if (!unit || unit->IsPlayer())
            {
                continue;
            }
            if (unit->GetID() == deader)
            {
                RmvCombatUnit(unit);
                continue;
            }

            //怪物死亡时调用队友AI脚本
            CombatNpc* mob = dynamic_cast<CombatNpc*>(unit);
            mob->OnTeammateDead();
        }
    }
}

void CombatPVE::OnHeartBeat()
{
	//do nothing
}

bool CombatPVE::OnMessageHandler(const MSG& msg)
{
    switch (msg.message)
    {
        case COMBAT_MSG_COMBAT_CONTINUE:
            {
                MsgHandler_CombatContinue(msg);
            }
            break;
        case COMBAT_MSG_RESUME_SCENE_SCRIPT:
            {
                MsgHandler_ResumeSceneScript(msg);
            }
            break;
        case COMBAT_MSG_TRIGGER_TRANSFORM:
            {
                MsgHandler_TriggerTransform(msg);
            }
            break;
        case COMBAT_MSG_START_TRANSFORM:
            {
                MsgHandler_StartTransform(msg);
            }
            break;
        case COMBAT_MSG_TRIGGER_ESCAPE:
            {
                MsgHandler_TriggerEscape(msg);
            }
            break;
        case COMBAT_MSG_START_ESCAPE:
            {
                MsgHandler_StartEscape(msg);
            }
            break;
        default:
            {
                return false;
            }
            break;
    };
	return true;
}

void CombatPVE::MsgHandler_CombatContinue(const MSG& msg)
{
	//继续下一场战斗
	assert(attacker_win_);

	///
	/// 开始新一波战斗
	///

	++ wave_counter_;

    //清除ATBMan状态
    atb_man_.Clear();

	//清除场景Lua栈
	if (scene_ai_)
	{
		scene_ai_->Release();
	}

	//清除怪物对象
	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* obj = defender_list_[i];
		if (obj && obj->IsNPC())
		{
			shared::MutexLockGuard keeper(obj->GetMutex());
            recycle_list_.push_back(obj);
			//CombatNpc* npc = dynamic_cast<CombatNpc*>(obj);
			//s_pCombatMan->RemoveNPCFromMan(npc);
			//s_pCombatMan->FreeNPC(npc);
			defender_list_[i] = NULL;
		}
	}

	//计算存活玩家个数
	int32_t alive_player_count = 0;
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (unit && unit->IsAlive())
        {
			++ alive_player_count;
            unit->ResetState();
        }
	}

	//生成新怪物组
	std::vector<MobInfo> list;
	if (!s_mob_spawner.GenerateMob(mob_group_id_, list, alive_player_count))
	{
		assert(false && "不可能出现的错误!");
		return;
	}

	//创建战斗怪
	for (size_t i = 0; i < list.size(); ++ i)
	{
		CombatNpc* npc = CreateCombatNpc(list[i], CombatNpc::TYPE_NPC_MOB);
		if (!npc) return; //战斗结束
		if (!AddCombatUnit(npc))
		{
            recycle_list_.push_back(npc);
			//s_pCombatMan->RemoveNPCFromMan(npc);
			//s_pCombatMan->FreeNPC(npc);
			npc->Unlock();
			return;
		}
		npc->Unlock();
	}

	//将怪物信息同步给玩家
	G2C::CombatPVEContinue packet;
	packet.wave_count = wave_counter_;
	packet.wave_common_hint_id = s_mob_spawner.GetCombatCommonHintID(mob_group_id_);
	packet.wave_start_hint_id = s_mob_spawner.GetCombatStartHintID(mob_group_id_);
	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (unit)
		{
			assert(unit->IsMob());
			G2C::CombatMobInfo minfo;
			CombatNpc* mob = dynamic_cast<CombatNpc*>(unit);
			mob->SaveForClient(minfo);
			packet.mob_list.push_back(minfo);
		}
	}
	BroadCastCMD(packet);

    //重置战场状态
    combat_state_.Reset();

    //重置场景Lua栈
    if (scene_ai_)
    {
		scene_ai_->Init();
    }

	__PRINTF("----------------------下一波战斗开始----------------------");
	Trace();
}

void CombatPVE::MsgHandler_ResumeSceneScript(const MSG& msg)
{
	if (!scene_ai_)
    {
        assert(false);
		return;
    }

	/*if (scene_ai_->Resume())
    {
        //脚本执行完了, 
        if (IsStateSuspend() || IsStateWaitSelectSkill())
        {
            //战斗仍然处于暂停状态则主动恢复战斗
            combat_state_.Update(XEVENT_RESUME);
        }
        else
        {
            //战斗应该是结束了
            assert(combat_state_.IsStateCloseWait());
        }
    }*/
    //脚本执行完了
    scene_ai_->Resume();
    /*if (IsStateSuspend() || IsStateWaitSelectSkill())
    {
        //战斗仍然处于暂停状态则主动恢复战斗
        //combat_state_.Update(XEVENT_RESUME);
        scene_ai_->Resume();
    }
    else
    {
        //战斗应该是结束了
        //assert(combat_state_.IsStateCloseWait());
    }*/
}

void CombatPVE::MsgHandler_TriggerTransform(const MSG& msg)
{
    UnitID  mob_unit_id = msg.source.id;
    TemplID new_mob_tid = msg.param;

    if (!s_mob_man.IsValidMob(new_mob_tid))
        return;

    CombatUnit* unit = Find(mob_unit_id);
    if (!unit || !unit->IsMob())
		return;

    CombatNpc* mob = dynamic_cast<CombatNpc*>(unit);
    if (!mob->CanTransform())
    {
		__PRINTF("战斗对象 %d 变身失败!!!", unit->GetID());
        return;
    }

    mob->Transform(new_mob_tid);
}

void CombatPVE::MsgHandler_StartTransform(const MSG& msg)
{
    UnitID mob_unit_id  = msg.source.id;
    TemplID new_mob_tid = msg.param;

    CombatUnit* unit = Find(mob_unit_id);
    if (!unit || !unit->IsMob())
		return;

    int mob_pos = unit->GetPos();
    if (!RmvCombatUnit(unit))
        return;

	NpcInfo mob_info;
	mob_info.id  = new_mob_tid;
	mob_info.pos = mob_pos;
	mob_info.lvl = s_mob_man.GetMobLevel(new_mob_tid);
	CombatNpc* mob = CreateCombatNpc(mob_info, CombatNpc::TYPE_NPC_MOB);
	if (!mob)
        return;

	if (!AddCombatUnit(mob))
	{
        recycle_list_.push_back(mob);
		//s_pCombatMan->RemoveNPCFromMan(mob);
		//s_pCombatMan->FreeNPC(mob);
		mob->Unlock();
        return;
	}
	mob->Unlock();

    //更新为变身状态
    mob->UpdateState(EVENT_TRANSFORMING, MONSTER_TRANSFORM_TIME);

    //广播给玩家
    G2C::CombatMobTransform packet;
    packet.unit_id = mob_unit_id;
    mob->SaveForClient(packet.new_mob_info);
    BroadCastCMD(packet);
}

void CombatPVE::MsgHandler_TriggerEscape(const MSG& msg)
{
    UnitID  mob_unit_id = msg.source.id;
    CombatUnit* unit = Find(mob_unit_id);
    if (!unit || !unit->IsMob())
		return;

    CombatNpc* mob = dynamic_cast<CombatNpc*>(unit);
    if (!mob->CanEscape())
    {
		__PRINTF("战斗对象 %d 逃跑失败!!!", unit->GetID());
        return;
    }

    // 设置强制战斗结果
    force_combat_result_ = msg.param;

    //广播给玩家
    G2C::CombatMobEscape packet;
    packet.unit_id = mob_unit_id;
    packet.mob_pos = mob->GetPos();
    BroadCastCMD(packet);

    mob->Escape();
}

void CombatPVE::MsgHandler_StartEscape(const MSG& msg)
{
    UnitID mob_unit_id  = msg.source.id;

    CombatUnit* unit = Find(mob_unit_id);
    if (!unit || !unit->IsMob() || !unit->TestEscaping())
		return;

    if (!RmvCombatUnit(unit))
        return;

    TestCombatEnd();
}

void CombatPVE::MobSpeak(UnitID mob_uid, int id_talk, int msec)
{
	CombatUnit* unit = Find(mob_uid);
	if (!unit)
	{
		__PRINTF("非法unit_id: %d, combat_id: %d", mob_uid, xid_.id);
		assert(false);
		return;
	}

	unit->Speak(id_talk, msec);
}

void CombatPVE::MultiMobSpeak(TemplID mob_tid, int id_talk, int msec)
{
	G2C::CombatMultiMobSpeak packet;
	packet.mob_tid   = mob_tid;
	packet.talk_id   = id_talk;
	packet.talk_time = msec;
	BroadCastCMD(packet);
}

void CombatPVE::PlayerSpeak(int id_talk, int msec)
{
    for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (unit && unit->IsPlayer())
		{
			G2C::CombatUnitSpeak packet;
			packet.unit_id   = unit->GetID();
			packet.talk_id   = id_talk;
			packet.talk_time = msec;
			BroadCastCMD(packet);
		}
	}
}

void CombatPVE::PlayerActivateSkill(int id_skgrp, int lvl_skgrp, int msec)
{
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (unit && unit->IsPlayer())
		{
			G2C::CombatLearnSkill packet;
			packet.skill_group_id = id_skgrp;
			packet.skill_group_level = lvl_skgrp;
			packet.time_learn = msec;
			BroadCastCMD(packet);
		}
	}
}

void CombatPVE::SummonMob(int mob_type, TemplID mob_tid, int pos)
{
	if (GetMobCount(mob_type == CombatNpc::TYPE_NPC_MOB) >= MAX_COMBAT_UNIT_NUM)
	{
		return;
	}

	MobInfo mob_info;
	mob_info.id = mob_tid;
	mob_info.pos = pos;
	mob_info.lvl = s_mob_man.GetMobLevel(mob_tid);
	CombatNpc* npc = CreateCombatNpc(mob_info, mob_type);
	if (!npc)
	{
		return;
	}

	if (!AddCombatUnit(npc))
	{
        recycle_list_.push_back(npc);
		//s_pCombatMan->RemoveNPCFromMan(npc);
		//s_pCombatMan->FreeNPC(npc);
		npc->Unlock();
        return;
	}
	npc->Unlock();

	npc->RegisterATB();

    G2C::CombatSummonMob packet;
    packet.mob_type = mob_type;
    packet.mob_pos  = npc->GetPos();
    packet.mob_tid  = npc->GetNpcID();
    packet.mob_hp   = npc->GetHP();
    packet.unit_id  = npc->GetID();
    BroadCastCMD(packet);
}

void CombatPVE::CastInstantSkill(TemplID mob_tid, SkillID skill_id)
{
	CombatUnitVec list;
	FindMob(mob_tid, list);

	for (size_t i = 0; i < list.size(); ++ i)
	{
		CombatNpc* mob = dynamic_cast<CombatNpc*>(list[i]);
		mob->SetSkill(skill_id);
		RegisterInstantATB(mob);
	}
}

void CombatPVE::Shake(int x_amplitude, int y_amplitude, int duration, int interval)
{
	G2C::CombatSceneShake packet;
    packet.x_amplitude = x_amplitude;
    packet.y_amplitude = y_amplitude;
    packet.shake_duration = duration;
    packet.shake_interval = interval;
    BroadCastCMD(packet);
}

int CombatPVE::GetUnitProp(int party, int pos, int prop, int& value)
{
    if (party != PARTY_ATTACKER && party != PARTY_DEFENDER)
    {
        return -1;
    }
    if (pos < 0 || pos > MAX_COMBAT_UNIT_NUM)
    {
        return -1;
    }
    if (prop < PROP_INDEX_MAX_HP || prop > PROP_INDEX_EP)
    {
        return -1;
    }
    CombatUnit* unit = party == PARTY_ATTACKER ? attacker_list_[pos] : defender_list_[pos];
    value = unit == NULL || unit->IsDead() ? 0 : unit->GetProp(prop);
    return 0;
}

int CombatPVE::GetUnitType(UnitID uid)
{
    for (int32_t i = 0; i < 2; ++i)
    {
        CombatUnitVec& unit_list = i == 0 ? attacker_list_ : defender_list_;
        for (size_t j = 0; j < unit_list.size(); ++j)
        {
            CombatUnit* unit = unit_list[j];
            if (unit == NULL || unit->GetID() != uid)
            {
                continue;
            }
            if (unit->IsPlayer())
            {
                return UT_PLAYER;
            }
            CombatNpc* npc = dynamic_cast<CombatNpc*>(unit);
            if (npc->IsMob())
            {
                return UT_MOB;
            }
            else if (npc->IsPet())
            {
                return UT_PET;
            }
            else if (npc->IsGolem())
            {
                return UT_GOLEM;
            }
            else if (npc->IsTeamNpc())
            {
                return UT_TEAM_NPC;
            }
            else if (npc->IsBoss())
            {
                return UT_BOSS;
            }
        }
    }
    return UT_INVALID;
}

void CombatPVE::Trace() const
{
#ifdef _COMBAT_DUMP
	Combat::Trace();

	__PRINTF("怪物在大世界的ID: %d", world_monster_id_);
	__PRINTF("怪物组ID：%d, 下一波怪物组ID：%d", mob_group_id_, next_mob_group_id_);
	__PRINTF("第%d波/共%d波", wave_counter_, wave_total_);
	__PRINTF("世界BOSS在战斗中的ID: %d", world_boss_id_);
	__PRINTF("当前玩家个数：%d", init_player_count_);
	if (attacker_sneaked_)
		__PRINTF("玩家被偷袭");
	if (defender_sneaked_)
		__PRINTF("怪物被偷袭");
#endif
}

/****************************CombatPVP********************************/
/****************************CombatPVP********************************/
/****************************CombatPVP********************************/
/****************************CombatPVP********************************/
CombatPVP::CombatPVP(): Combat(OBJ_TYPE_COMBAT_PVP),
    combat_flag_(0)
{
}

CombatPVP::~CombatPVP()
{
}

bool CombatPVP::OnInit()
{
    return true;
}

void CombatPVP::OnHeartBeat()
{
}

void CombatPVP::OnCombatStart()
{
	//战斗正式开始了

    //检查对方玩家是否成功加入战斗
    //如果加入失败，则强制结束战斗
    bool empty1 = true;
    bool empty2 = true;
    for (size_t i = 0; i < attacker_list_.size(); ++ i)
    {
        if (attacker_list_[i])
        {
            empty1 = false;
        }
    }
    for (size_t i = 0; i < defender_list_.size(); ++ i)
    {
        if (defender_list_[i])
        {
            empty2 = false;
        }
    }

    if (empty1 || empty2)
    {
        //战场中有一方是空的。。。
        //强制结束战斗
        //DoCombatEnd();
        //return;
    }

	//注册ATB
	RegisterAllATB();

	//执行场景AI策略
	if (scene_ai_)
	{
		scene_ai_->OnCombatStart();
	}

	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit) continue;
		unit->CombatStart();
	}

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (!unit) continue;
		unit->CombatStart();
	}
}

void CombatPVP::OnCombatEnd()
{
    //TODO
}

void CombatPVP::DoCombatEnd()
{
	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit)
            continue;

        CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);

		CombatPVPResult result;
		result.result = attacker_win_ ? RESULT_WIN : RESULT_FAIL;
        result.combat_flag = combat_flag_;
        result.combat_creator = creator_roleid_;
        result.hp = player->GetHP();
		result.pet_power = player->GetPetPower();
        result.player_killed_vec = player->GetPlayersKilled();

		//同步战斗结果给GS
		Combat::CombatPVPResultCB(player->GetRoleID(), xid_.id, player->GetID(), result, player->GetMasterServerID());

		//通知玩家战斗结束
        Combat::CombatPVPEndCB(player->GetRoleID(), 0, xid_.id, attacker_win_);
    }

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (!unit)
            continue;

        CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);

		CombatPVPResult result;
		result.result = attacker_win_ ? RESULT_FAIL : RESULT_WIN;
        result.combat_flag = combat_flag_;
        result.combat_creator = creator_roleid_;
        result.hp = player->GetHP();
		result.pet_power = player->GetPetPower();
        result.player_killed_vec = player->GetPlayersKilled();

		//同步战斗结果给GS
		Combat::CombatPVPResultCB(player->GetRoleID(), xid_.id, player->GetID(), result, player->GetMasterServerID());

		//通知玩家战斗结束
        Combat::CombatPVPEndCB(player->GetRoleID(), 0, xid_.id, attacker_win_);
    }

	s_pCombatMan->RemovePVPCombatFromMan(this);
	s_pCombatMan->FreePVPCombat(this);

	__PRINTF("----------------------销毁PVP战场(id=%d)----------------------", xid_.id);
}

bool CombatPVP::CanJoinCombat() const
{
    return IsStateOpen();
}

void CombatPVP::OnSomeoneDead(UnitID deader, UnitID killer)
{
    //TODO
}

void CombatPVP::Clear()
{
	Combat::Clear();

    combat_flag_ = 0;
}

void CombatPVP::OnStartCombat()
{
	//将战场信息同步给客户端
	G2C::CombatPVPStart packet;
    BuildCombatData(packet);
    packet.which_party = packet.attacker_list[0].which_party;
	BroadCastCMD(packet);
}

void CombatPVP::BuildCombatData(shared::net::ProtoPacket& packet)
{
	G2C::CombatPVPStart* pPacket = dynamic_cast<G2C::CombatPVPStart*>(&packet);
	pPacket->combat_id = xid_.id;
	pPacket->combat_scene_id = scene_id_;

	for (size_t i = 0; i < attacker_list_.size(); ++ i)
	{
		CombatUnit* unit = attacker_list_[i];
		if (!unit) continue;
		if (unit->IsPlayer())
		{
			G2C::CombatPlayerInfo pinfo;
			CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);
			player->SaveForClient(pinfo);
			pPacket->attacker_list.push_back(pinfo);
		}
	}

	for (size_t i = 0; i < defender_list_.size(); ++ i)
	{
		CombatUnit* unit = defender_list_[i];
		if (!unit) continue;
		if (unit->IsPlayer())
		{
			G2C::CombatPlayerInfo pinfo;
			CombatPlayer* player = dynamic_cast<CombatPlayer*>(unit);
			player->SaveForClient(pinfo);
			pPacket->defender_list.push_back(pinfo);
		}
	}
}

bool CombatPVP::OnMessageHandler(const MSG& msg)
{
    return true;
}

void CombatPVP::BroadCastCMD(shared::net::ProtoPacket& packet, UnitID ex_id)
{
	DoBroadCastCMD(attacker_list_, packet, ex_id);
	DoBroadCastCMD(defender_list_, packet, ex_id);
}

void CombatPVP::Trace() const
{
#ifdef _COMBAT_DUMP
	Combat::Trace();

	if (attacker_sneaked_)
		__PRINTF("攻方被偷袭");
	if (defender_sneaked_)
		__PRINTF("守方被偷袭");
#endif
}

};
