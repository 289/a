#include <assert.h>
#include <string.h>
#include "combat_unit.h"
#include "combat_msg.h"
#include "combat_obj_if.h"
#include "combat.h"
#include "combat_man.h"
#include "combat_def.h"
#include "time_def.h"
#include "combat_npc.h"
#include "combat_player.h"
#include "mob_manager.h"

#include "obj_if/obj_interface.h"
#include "client_proto/G2C_proto.h"
#include "skill/include/skill_info.h"
#include "skill/include/skill_types.h"
#include "skill/include/cooldown_wrapper.h"

namespace combat
{

#define OBJIF_2_COMBAT_UNIT(obj)  (dynamic_cast<CombatObjInterface*>(obj)->GetParent())

CombatUnit::CombatUnit(char type): Object(type),
	  pos_(-1),
      party_(0),
	  level_(0),
	  state_(0),
	  skill_(NULL),
	  buff_man_(NULL),
	  skill_cd_(NULL),
	  obj_if_(NULL),
	  refresh_extprop_mask_(0),
	  refresh_volatile_prop_(false),
	  refresh_basic_prop_(false),
	  is_atb_owner_(false),
      is_transforming_(false),
      is_escaping_(false),
	  attack_done_(false),
	  killer_(0),
	  default_skill_(0),
	  recast_skill_(0),
	  round_counter_(0),
	  cur_golem_(NULL),
      golem_action_(false),
      refresh_cur_golem_(false),
      refresh_other_golem_(false),
	  combat_(NULL)
{
	memset(&basic_prop_, 0, sizeof(basic_prop_));
	memset(&base_prop_, 0, sizeof(base_prop_));
	memset(&cur_prop_, 0, sizeof(cur_prop_));
	memset(&enh_point_, 0, sizeof(enh_point_));
	memset(&enh_scale_, 0, sizeof(enh_scale_));
}

CombatUnit::~CombatUnit()
{
	if (skill_)
	{
		delete skill_;
		skill_ = NULL;
	}

	if (buff_man_)
	{
		delete buff_man_;
		buff_man_ = NULL;
	}

	if (skill_cd_)
	{
		delete skill_cd_;
		skill_cd_ = NULL;
	}

	if (obj_if_)
	{
		delete obj_if_;
		obj_if_ = NULL;
	}
}

void CombatUnit::Initialize()
{
	if (!obj_if_)
		obj_if_ = new CombatObjInterface(this);

	if (!skill_)
		skill_ = new skill::SkillWrapper(obj_if_);

	if (!buff_man_)
		buff_man_ = new skill::BuffWrapper(obj_if_);

	if (!skill_cd_)
		skill_cd_ = new skill::CooldownWrapper(obj_if_);

	OnInit();
}

void CombatUnit::Die()
{
	skill::BuffDamageVec damages;
	buff_man_->Dead(damages);
	SendBuffResult(damages);

	OnDeath();

	//通知战场有战斗对象死亡
	MSG msg;
	BuildMessage(msg, COMBAT_MSG_SOMEONE_DEAD, combat_->GetXID(), xid_, killer_, NULL, 0);
	combat_->SendMSG(msg);
}

void CombatUnit::Dying()
{
	if (cur_golem_)
	{
		cur_golem_->OnDeActived();
	}
	OnDying();
}

void CombatUnit::Clear()
{
	Object::Clear();

	memset(&basic_prop_, 0, sizeof(basic_prop_));
	memset(&base_prop_, 0, sizeof(base_prop_));
	memset(&cur_prop_, 0, sizeof(cur_prop_));
	memset(&enh_point_, 0, sizeof(enh_point_));
	memset(&enh_scale_, 0, sizeof(enh_scale_));

	if (skill_)
	{
		delete skill_;
		skill_ = NULL;
	}

	if (buff_man_)
	{
		delete buff_man_;
		buff_man_ = NULL;
	}

	if (skill_cd_)
	{
		delete skill_cd_;
		skill_cd_ = NULL;
	}

	if (obj_if_)
	{
		delete obj_if_;
		obj_if_ = NULL;
	}

	//释放战斗魔偶
	if (cur_golem_)
	{
        combat_->RmvGolem(cur_golem_);
		s_pCombatMan->RemoveNPCFromMan(cur_golem_);
		s_pCombatMan->FreeNPC(cur_golem_);
	}

	//释放休息魔偶
	for (size_t i = 0; i < golem_list_.size(); ++ i)
	{
		CombatNpc* golem = golem_list_[i];
        combat_->RmvGolem(golem);
		s_pCombatMan->RemoveNPCFromMan(golem);
		s_pCombatMan->FreeNPC(golem);
	}

	pos_              = -1;
    party_            = 0;
	level_            = 0;
	state_            = 0;
	is_atb_owner_     = false;
    is_transforming_  = false;
    is_escaping_      = false;
	attack_done_      = false;
	killer_           = 0;
	default_skill_    = 0;
	recast_skill_     = 0;
	round_counter_    = 0;
	cur_golem_        = NULL;
	combat_           = NULL;
    golem_action_     = false;

	ClrRefreshExtPropMask();
	ClrRefreshBasicProp();
	ClrRefreshVolatileProp();
    ClrRefreshCurGolem();
    ClrRefreshOtherGolem();

	unit_state_.Clear();
	golem_list_.clear();
	target_list_.clear();
	buff_damages_.clear();
	skill_damages_.clear();
} 

void CombatUnit::OnSendCurGolemProp() const
{
    G2C::CombatUnitCurGolemProp packet;
    packet.unit_id = GetID();
    packet.golem_id = cur_golem_->GetNpcID();
    packet.mp = cur_golem_->GetProp(PROP_INDEX_MP);
    BroadCastCMD(packet);
}

void CombatUnit::OnSendOtherGolemProp() const
{
    G2C::CombatUnitOtherGolemProp packet;
    CombatNpcVec::const_iterator it = golem_list_.begin();
    for (; it != golem_list_.end(); ++it)
    {
        CombatNpc* golem = *it;
        int32_t golem_id = golem->GetNpcID();
        packet.cur_power[golem_id] = golem->GetProp(PROP_INDEX_MP); 
        packet.deactive_power[golem_id] = golem->GetDeActivePower(); 
    }
    SendPacket(packet);
}

void CombatUnit::HeartBeat()
{
	//尝试刷新属性
	RefreshExtendProp();

	unit_state_.HeartBeat(this);

	OnHeartBeat();
}

void CombatUnit::RegisterATB()
{
	if (IsATBOwner() && IsAlive() && !TestTransforming())
	{
		combat_->RegisterATB(this);
	}
}

void CombatUnit::RegisterInstantATB()
{
	if (IsATBOwner() && IsAlive())
	{
		combat_->RegisterInstantATB(this);
	}
}

void CombatUnit::ResetATB()
{
	if (IsATBOwner() && IsAlive())
	{
		combat_->ResetATB(this);
	}
}

void CombatUnit::UnRegisterATB()
{
	combat_->UnRegisterATB(xid_.id);
}

bool CombatUnit::StartAttack()
{
	//魔偶共享主人的ATB
	//只有主人和魔偶的攻击都完成才删除主人的ATB
    if (!golem_action_)
    {
        //对象攻击
        if (!TestAttackDone())
        {
            if (Attack())
            {
                SetAttackDone();
            }
        }

        //测试攻击是否已经全部完成
        if (!TestAttackDone()) return false;

        //攻击全部完成,
        //清除攻击完成标记
        ClrAttackDone();
        return true;
    }
    else
    {
        return GolemCast();
    }
}

bool CombatUnit::Attack()
{
	//处理偷袭
	if (!CheckSneaked())
	{
		//处于偷袭状态，忽略本回合攻击
		return true;
	}

    if (!IsPet() && !IsGolem() && combat_->NeedReviseATB(this))
    {
        return false;
    }

	//检查自身状态
	if (!CanAction())
	{
        if (IsAction() || IsAttacked() || IsTransformWait() || IsTransforming())
		{
			combat_->DeActivateATB(GetStateTimeout());
			return false;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//处理自身的特殊状态
	if (IsSleep() || IsDizzy() || IsCharging())
	{
		//处于眩晕，睡眠，蓄力状态时，忽略本回合攻击，
		UpdateState(EVENT_ACTION, 100/*ms*/);
		return true;
	}

	//获取技能ID
	bool recast = true;
	SkillID skillid = GetRecastSkill();
	if (!skillid)
	{
		recast  = false;
		skillid = GetSkill();
	}
	//assert(skillid > 0);

	//开始攻击
	while (skillid > 0)
	{
		if (!DoAttack(skillid, recast))
		{
			return false;
		}

		skillid = GetRecastSkill();
		if (skillid > 0)
		{
			recast = true;
		}
	};

	//攻击结束
	AttackEnd();

	//test
	//回合结束所有对象状态被正确设置
    combat_->AssertZombie();

	return true;
}

bool CombatUnit::DoAttack(SkillID skillid, bool recast)
{
	assert(CanAction());

	//处理XP技能
	int atb_stop_time = skill::GetATBStopTime(skillid);
	if (atb_stop_time > 0)
	{
		//施放XP技能时所有人都处于战斗站立
		if (combat_->HasSomeoneAction())
			return false;
	}

	//获取攻击目标
	ObjIfVec targets;
	int attack_pos = skill_->GetTarget(skillid, targets);
	if (attack_pos == skill::POS_INVALID)
	{
		ClrRecastSkill();
		return true;
		//if (!recast)
		//{
			//__PRINTF("GetTarget failed. attacker=%d, skillid=%d", xid_.id, skillid);
			//assert(false);
			//return false;
		//}
		//else
		//{
		//}
	}

	//检查目标状态
    ObjIfVec::iterator it = targets.begin();
    for (; it != targets.end();)
    {
		CombatUnit* target = OBJIF_2_COMBAT_UNIT(*it);
		if (IsEnemy(target->GetID()) && !target->CanAttacked())
		{
            if (target->IsAction() || target->IsTransforming())
            {
                int32_t tick = target->GetStateTimeout();
                __PRINTF("-----------------战斗对象暂停战斗(%d), 持续%dmsec!!!", target->GetID(), tick * MSEC_PER_TICK);
                combat_->DeActivateATB(target->GetStateTimeout());
            }
            else if (target->IsTransformWait())
            {
                //这里的1个TICK为时间补偿
                combat_->DeActivateATB(target->GetStateTimeout() + (MONSTER_TRANSFORM_TIME/MSEC_PER_TICK) + 1);
            }
            else if (target->IsDead())
            {
                __PRINTF("-----------------战斗对象(%d), 已经死亡!!!", target->GetID());
                it = targets.erase(it);
                continue;
            }
            else
            {
                assert(false);
            }
            return false;
		}
        ++it;
    }
	for (size_t i = 0; i < targets.size(); ++ i)
	{
	}

	//缓存攻击目标
	for (size_t i = 0; i < targets.size(); ++ i)
	{
		CombatUnit* target = OBJIF_2_COMBAT_UNIT(targets[i]);
		if (IsEnemy(target->GetID()))
		{
			target_list_.push_back(target);
		}
	}

	//回合开始
	if (!recast && !RoundStart())
	{
		//回合开始会产生buff, buff可能导致自己或者对方死亡。
		//战斗结束或攻击者已死亡则本回合攻击就直接结束了。
		return true;
	}
    if (TestEscaping())
    {
        // 如果逃跑了则本回合就直接结束了
        return true;
    }

	//施放技能
	skill::SkillDamage damage;
	skill_->CastSkill(skillid, targets, damage, recast);

	skill::BuffDamageVec buff_damages;
	SkillID skill_recast = buff_man_->CastSkill(damage, buff_damages);
	damage.cast_pos = attack_pos;
	skill_damages_.push_back(damage);
	SaveBuffMessage(buff_damages);
	SetRecastSkill(skill_recast);

	OnAttack();

	if (recast)
	{
		__PRINTF("----------------------施放追击技能, skillid=%d------------------------", skillid);
	}

	char buf[1024];
	memset(&buf, 0, sizeof(buf) / sizeof(char));
	sprintf((char*)(buf), "----------------------产生技能攻击消息: 技能ID=%d, 攻击者=%ld, 被攻击者(", (int)(damage.skillid), damage.attacker);
	for (size_t i = 0; i < targets.size(); ++ i)
		sprintf((char*)(buf+strlen(buf)), "%d, ", targets[i]->GetId());
	sprintf((char*)(buf+strlen(buf)), ")");
	__PRINTF("%s", buf);

	return true;
}

bool CombatUnit::CheckSneaked()
{
	if (IsSneaked())
	{
		//处于被偷袭状态，则肯定是第一次攻击
		//所以这里被偷袭状态超时。
		//UpdateState(EVENT_TIMEOUT);

		//广播战斗对象转身
		G2C::CombatUnitTurnFront packet;
		packet.unit_id = GetID();
		BroadCastCMD(packet);

		//被偷袭超时回调
		OnSneakedEnd();
		return false;
	}

	return true;
}

bool CombatUnit::RoundStart()
{
	skill::BuffDamageVec damages;
	buff_man_->RoundStart(damages);

	if (IsZombie())
	{
		//buff导致自己死亡，本次攻击失败.
		SendBuffResult(damages);

		//这里需要纠正战斗对象的状态持续时间，因为在OnDamage的设置为-1
		//尽管技能施放失败，但是客户端需要播放BUFF掉血，暂时把播放时间定为1000ms
		//1000ms后执行战斗对象正常死亡逻辑

		SetStateTimeout(MAX_BUFF_PLAY_TIME);

		__PRINTF("技能释放者 %d 在回合开始由于BUFF掉血导致死亡", xid_.id);

		return false;
	}

	++ round_counter_;

	SaveBuffMessage(damages);

	for (size_t i = 0; i < golem_list_.size(); ++ i)
	{
		//回合开始给休战魔偶恢复能量
		CombatNpc* golem = golem_list_[i];
		golem->RestorePower();
        SetRefreshOtherGolem();
	}
    if (cur_golem_ != NULL)
    {
        cur_golem_->RoundStart();
    }

	OnRoundStart();

    if (!IsGolem() && !IsPet())
    {
	    combat_->OnRoundStart(xid_.id);
    }

	return true;
}

void CombatUnit::AttackEnd()
{
	OnAttackEnd();
    // 主要是同步能量消耗的变化
	UpdateAndSendProp();

	//获取施放和受击时间
	skill::PauseTime time_map;
	skill::GetPauseTime(model_, skill_damages_, time_map);

	///
	/// 更新攻击者状态
	///
    //int32_t id = obj_if_->GetId();
	skill::PauseTime::const_iterator it_self = time_map.find(GetID());
	if (it_self != time_map.end())
	{
		int32_t action_time = it_self->second;
		if (IsZombie() || IsZombieDying())
		{
			//攻击过程中自己死亡，死亡状态在DoDamage中已设置，这里只需设置STATUS_ZOMBIE状态的持续时间
			SetStateTimeout(action_time);
		}
		else
		{
			//更新为行动状态
            OnUpdateAttackState(action_time);
            __PRINTF("战斗对象 %d 更新战斗状态：time: %d", xid_.id, action_time);

            //__PRINTF("战斗对象 %d 进入攻击状态：time: %ld", xid_.id, GetSysTimeMsec());
		}
	}

	///
	/// 更新受击者状态
	///
	for (size_t i = 0; i < target_list_.size(); ++ i)
	{
		UnitID target_id = target_list_[i]->GetID();
		if (target_id == GetID())
		{
			//目标为自己，自己的状态上面已经更新了
			//注意：攻击者有时候也可能作为目标存在的。
			continue;
		}
		else if (IsTeammate(target_id))
		{
			//目标为队友，无需更新状态
			continue;
		}
		else if (IsEnemy(target_id))
		{
			//目标为敌人，更新为受击状态
			skill::PauseTime::const_iterator it_target = time_map.find(target_id);
			if (it_target != time_map.end())
			{
				CombatUnit* target = combat_->Find(target_id);
                int32_t attacked_time = it_target->second;
				target->OnAttacked(attacked_time);
			}
		}
        else
        {
            assert(false);
        }
	}

	combat_->BroadcastSkillResult(this, skill_damages_);
	combat_->BroadcastBuffResult(buff_damages_);

	//处理XP技能, XP技能开始后暂定ATB
	for (size_t i = 0; i < skill_damages_.size(); ++ i)
	{
		SkillID __skill = skill_damages_[i].skillid;
		int atb_stop_time = skill::GetATBStopTime(__skill);
		if (atb_stop_time > 0)
		{
            int tick = atb_stop_time / MSEC_PER_TICK;
			combat_->DeActivateATB(tick);
		}
	}

	//更新属性并同步客户端
    // 暂时去掉，在行动结束以后再同步 LJH
	//UpdateAndSendProp();

	target_list_.clear();
	buff_damages_.clear();
	skill_damages_.clear();
}

void CombatUnit::OnUpdateAttackState(int32_t action_time)
{
	UpdateState(EVENT_ACTION, action_time);
}

void CombatUnit::RoundEnd()
{
    if (golem_action_)
    {
        cur_golem_->RoundEnd();
    }
    // 没有魔偶，或者魔偶能量不足的，玩家行动
    if (cur_golem_ == NULL || cur_golem_->GetProp(PROP_INDEX_MP) <= 0)
    {
        golem_action_ = false;
    }
    else
    {
        // 有魔偶且魔偶还可以行动的，设置成魔偶行动并注册ATB
        if (!golem_action_)
        {
            combat_->RegisterInstantATB(this);
        }
        golem_action_ = !golem_action_;
    }

	OnRoundEnd();

	combat_->OnRoundEnd(xid_.id);

	//回合结束调用Buff
	skill::BuffDamageVec damages;
	buff_man_->RoundEnd(skill_damages_, damages);

    if (!skill_damages_.empty())
    {
        combat_->BroadcastSkillResult(this, skill_damages_);
        skill_damages_.clear();
    }
	SendBuffResult(damages);

	//回合结束重新注册ATB
	RegisterATB();

	//更新属性并同步客户端
	UpdateAndSendProp();

    __PRINTF("战斗对象 %d 离开攻击状态，time: %ld", xid_.id, GetSysTimeMsec());
}

void CombatUnit::OnSneakedEnd()
{
	//被偷袭状态结束
	//1)重置ATB
	ResetATB();

    UpdateState(EVENT_TIMEOUT);
    
	//2清除战场被偷袭标记
	combat_->ClrSneakedState();
}

void CombatUnit::TransformWaitEnd()
{
    OnTransformWaitEnd();
}

void CombatUnit::TransformingEnd()
{
    RegisterATB();
}

void CombatUnit::EscapeWaitEnd()
{
    OnEscapeWaitEnd();
}

bool CombatUnit::GolemCast()
{
    __PRINTF("******CombatUnit::GolemCast()******");
    // 有可能对象攻击后战斗已经结束
    // 需要提前检查，否则魔偶攻击时可能找不到目标
    if (combat_->IsCombatEnd() != 0)
    {
        return true;
    }

    if (!CanAction())
    {
        if (IsAction() || IsAttacked())
		{
			combat_->DeActivateATB(GetStateTimeout());
			return false;
		}
		else
		{
			assert(false);
			return false;
		}
    }
	//处理自身的特殊状态
	if (IsSleep() || IsDizzy() || IsCharging())
	{
		//处于眩晕，睡眠，蓄力状态时，忽略本回合攻击，
		UpdateState(EVENT_ACTION, 100/*ms*/);
		return true;
	}

	//魔偶攻击
    if (cur_golem_ && cur_golem_->GetProp(PROP_INDEX_MP) <= 0)
    {
        // 魔偶能量不足，忽略本回合
		UpdateState(EVENT_ACTION, 100/*ms*/);
		return true;
    }
	if (cur_golem_ && !cur_golem_->TestAttackDone())
	{
		if (cur_golem_->Attack())
		{
			cur_golem_->SetAttackDone();
		}
	}
	if (cur_golem_ && !cur_golem_->TestAttackDone()) return false;
	if (cur_golem_)
	{
		cur_golem_->ClrAttackDone();
	}
    return true;
}

void CombatUnit::OnAttacked(int attacked_time)
{
	if (IsZombie())
	{
		//已经死亡
		//死亡状态在DoDamage中已设置，这里只需设置STATUS_ZOMBIE状态的持续时间
		//受击时间超时后执行死亡逻辑
		SetStateTimeout(attacked_time);
	}
	else
	{
		//未死亡，
		if (IsSneaked() && (round_counter_ != 0))
		{
			//处于被偷袭状态，一旦受击，则解除被偷袭状态
			OnSneakedEnd();
		}
        else if (!IsAttacked())
		{
			//如果目标不处于受击状态时，则更新为被攻击状态，
			UpdateState(EVENT_ATTACKED, attacked_time);

            __PRINTF("战斗对象 %d 进入受击状态：time: %d", xid_.id, attacked_time);
		}
		else if ((GetStateTimeout() * MSEC_PER_TICK) < attacked_time)
		{
			//如果目标处于受击状态，并且超时时间小于新的受击时间时，则更新受击时间。
			SetStateTimeout(attacked_time);

            __PRINTF("战斗对象 %d 受击时间更新：time: %d", xid_.id, attacked_time);
		}
	}
}

void CombatUnit::AttackedEnd()
{
	//更新属性并同步客户端
	UpdateAndSendProp();

    OnAttackedEnd();

    __PRINTF("战斗对象 %d 离开受击状态：time: %ld", xid_.id, GetSysTimeMsec());
}

void CombatUnit::CombatStart()
{
	buff_man_->CombatStart(skill_damages_);
    if (!skill_damages_.empty())
    {
        combat_->BroadcastSkillResult(this, skill_damages_);
        skill_damages_.clear();
    }

	//更新属性并同步客户端
	UpdateAndSendProp();

    //子类函数
    OnCombatStart();
}

void CombatUnit::OnCombatEnd()
{
}

void CombatUnit::BroadCastCMD(shared::net::ProtoPacket& packet, UnitID ex_id) const
{
	combat_->BroadCastCMD(packet, ex_id);
}

void CombatUnit::AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher)
{
	combat_->AttachBuffer(buff_seq, buff_id, attacher, xid_.id);
}

void CombatUnit::DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher)
{
	combat_->DetachBuffer(buff_seq, buff_id, attacher, xid_.id);
}

gamed::ObjInterface* CombatUnit::GetObjIf()
{
	return obj_if_;
}

void CombatUnit::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		default:
		{
			//未知消息可能在子类处理
			OnMessageHandler(msg);
		}
		break;
	}
}

void CombatUnit::SetBasicProp(int32_t hp, int32_t mp, int32_t ep)
{
	basic_prop_.hp = hp > 0 ? hp : 0;
	basic_prop_.mp = mp > 0 ? mp : 0;
	basic_prop_.ep = ep > 0 ? ep : 0;
}

void CombatUnit::SetBaseProp(const std::vector<int32_t>& props)
{
	assert(props.size() == PROP_INDEX_HIGHEST);
	for (size_t i = 0; i < props.size(); ++ i)
	{
		base_prop_[i] = props[i];
		cur_prop_[i] = base_prop_[i];
	}
}

void CombatUnit::RefreshExtendProp()
{
	if (refresh_extprop_mask_)
	{
        //扩展属性发生变化
        PropPolicy::UpdateProperty(this);

        if (refresh_extprop_mask_ & PROP_MASK_MAX_HP)
        {
            //max_hp发生变化
            SetRefreshVolatileProp();
        }
	}
}

#define ASSERT_PROP_INDEX(idx)   {assert(idx >= 0 && idx < PROP_INDEX_HIGHEST);}
#define ASSERT_PROP_VALUE(value) {assert(value >= 0);}
//#define ASSERT_NOT_NPC           {assert(!IsPet() && !IsGolem());}
#define ASSERT_NOT_NPC           {assert(true);}
#define ASSERT_BOSS_UNIT         {assert(IsBoss());}

void CombatUnit::SyncHP(int new_hp)
{
    if (!IsBoss()) return;
	if (!IsAlive()) return;
	if (new_hp == GetHP()) return;

	int32_t old_hp = basic_prop_.hp;
	if (old_hp > 0 && new_hp <= 0)
	{
		basic_prop_.hp = 0;

		UpdateState(EVENT_ZOMBIE, GetStateTimeout() * MSEC_PER_TICK);

		if (!IsSneaked())
		{
			UnRegisterATB();
		}

        if (!combat_->IsStateClose())
        {
		    combat_->TestCombatEnd();
        }
	}
	else if (new_hp > GetMaxHP())
	{
		basic_prop_.hp = GetMaxHP();
	}
	else
	{
		basic_prop_.hp = new_hp;
	}
}

void CombatUnit::SetHP(int32_t new_hp)
{
	if (!IsAlive()) return;
	if (new_hp == GetHP()) return;

	if (new_hp > GetMaxHP())
	{
		basic_prop_.hp = GetMaxHP();
	}
	else
	{
		basic_prop_.hp = new_hp;
	}
}

void CombatUnit::DoHeal(int life, UnitID healer)
{
	ASSERT_NOT_NPC;

	if (life <= 0) return;
	if (IsDead()) return;

	basic_prop_.hp += life;
	if (basic_prop_.hp > GetMaxHP())
	{
		basic_prop_.hp = GetMaxHP();
	}
}

void CombatUnit::DoDamage(int damage, UnitID attacker)
{
	ASSERT_NOT_NPC;

	if (!IsAlive()) return;
	if (damage <= 0) return;

	int32_t old_hp = basic_prop_.hp;
	basic_prop_.hp -= damage;
	if (basic_prop_.hp <= 0 && killer_ == 0)
	{
		SetKiller(attacker);
	}

	if (old_hp > 0 && basic_prop_.hp <= 0)
	{
		UpdateState(EVENT_ZOMBIE);

		if (!IsSneaked())
		{
			UnRegisterATB();
		}

		combat_->TestCombatEnd();
	}

	OnDamage(damage, attacker);
}

void CombatUnit::IncEP(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;
	if (index != PROP_INDEX_EP)
		return;

	int32_t max_con2 = cur_prop_[PROP_INDEX_MAX_EP];
	basic_prop_.ep  += value;
	if (basic_prop_.ep >= max_con2)
	{
		basic_prop_.ep = max_con2;
	}

	SetRefreshBasicProp();
}

void CombatUnit::IncMP(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;
	if (index != PROP_INDEX_MP)
		return;

	int32_t max_con = cur_prop_[PROP_INDEX_MAX_MP];
	basic_prop_.mp += value;
	if (basic_prop_.mp >= max_con)
	{
		basic_prop_.mp = max_con;
	}

	__PRINTF("战斗对象 %d 获得能量： %d 总能量： %d", xid_.id, value, basic_prop_.mp);

	SetRefreshBasicProp();
}

void CombatUnit::DecMP(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;
	if (index != PROP_INDEX_MP)
		return;

	basic_prop_.mp -= value;
	if (basic_prop_.mp <= 0)
	{
		//OnPowerDrain();
		basic_prop_.mp = 0;
	}

	__PRINTF("战斗对象 %d 消耗能量： %d 总能量： %d", xid_.id, value, basic_prop_.mp);

	SetRefreshBasicProp();
}

void CombatUnit::DecEP(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;
	if (index != PROP_INDEX_EP)
		return;

	basic_prop_.ep -= value;
	if (basic_prop_.ep <= 0)
	{
		//OnPowerDrain();
		basic_prop_.ep = 0;
	}

	SetRefreshBasicProp();
}

void CombatUnit::IncProp(size_t index, int value)
{
	ASSERT_NOT_NPC;

	if (IsDead()) return;
	if (value <= 0) return;

	switch (index)
	{
		case PROP_INDEX_HP:
			assert(false && "此处不处理治疗,治疗由技能调用");
		break;
		case PROP_INDEX_MP:
			IncMP(index, value);
		break;
		case PROP_INDEX_EP:
			IncEP(index, value);
		break;
		default:
			IncPropPoint(index, value);
		break;
	}
}

void CombatUnit::DecProp(size_t index, int value)
{
	ASSERT_NOT_NPC;

	if (IsDead()) return;
	if (value <= 0) return;

	switch (index)
	{
		case PROP_INDEX_HP:
			assert(false && "此处不处理掉血,掉血由技能调用");
		break;
		case PROP_INDEX_MP:
			DecMP(index, value);
		break;
		case PROP_INDEX_EP:
			DecEP(index, value);
		break;
		default:
			DecPropPoint(index, value);
		break;
	}
}

void CombatUnit::IncPropPoint(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;

	enh_point_[index] += value;

	if (index == PROP_INDEX_ATB_TIME ||
		index == PROP_INDEX_ATTACK_PRIORITY)
	{
        combat_->OnATBChange(this);

        //__PRINTF("战斗对象 %d 的ATB点数增加, index: %ld, value: %d", xid_.id, index, value);

	}

	SetRefreshExtPropMask(index);
}

void CombatUnit::DecPropPoint(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;

	enh_point_[index] -= value;
    // 下面去掉因为攻战有个技能可以减少攻击速度，因此可以一上来就是负的
	//assert(enh_point_[index] >= 0);

	if (index == PROP_INDEX_ATB_TIME ||
		index == PROP_INDEX_ATTACK_PRIORITY)
	{
		combat_->OnATBChange(this);

        //__PRINTF("战斗对象 %d 的ATB点数减少, index: %ld, value: %d", xid_.id, index, value);
	}

	SetRefreshExtPropMask(index);
}

void CombatUnit::IncPropScale(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;

	enh_scale_[index] += value;
	SetRefreshExtPropMask(index);

	if (index == PROP_INDEX_ATB_TIME ||
		index == PROP_INDEX_ATTACK_PRIORITY)
	{
		combat_->OnATBChange(this);

        //__PRINTF("战斗对象 %d 的ATB比例增加, index: %ld, value: %d", xid_.id, index, value);
	}
}

void CombatUnit::DecPropScale(size_t index, int value)
{
	ASSERT_NOT_NPC;
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);

	if (IsDead()) return;
	if (value <= 0) return;

	enh_scale_[index] -= value;
	assert(enh_scale_[index] >= 0);

	SetRefreshExtPropMask(index);

	if (index == PROP_INDEX_ATB_TIME ||
		index == PROP_INDEX_ATTACK_PRIORITY)
	{
		combat_->OnATBChange(this);

        //__PRINTF("战斗对象 %d 的ATB比例减少, index: %ld, value: %d", xid_.id, index, value);
	}
}

int CombatUnit::GetProp(size_t index) const
{
	//if (IsDead()) return -1;  // 因为可能死了以后其上的Buff未消失
	if (index == PROP_INDEX_HP) return basic_prop_.hp;
	if (index == PROP_INDEX_MP) return basic_prop_.mp;
	if (index == PROP_INDEX_EP) return basic_prop_.ep;

	ASSERT_PROP_INDEX(index);
	return cur_prop_[index];
}

int CombatUnit::GetBaseProp(size_t index) const
{
	ASSERT_PROP_INDEX(index);
	//if (IsDead()) return -1;
	//return base_prop_[index];
    int value = base_prop_[index];
    return value < 0 ? -1 : value;
}
int CombatUnit::GetMaxProp(size_t index) const
{
	ASSERT_PROP_INDEX(index);
	if (IsDead()) return -1;
	return cur_prop_[index];
}

#undef ASSERT_PROP_INDEX
#undef ASSERT_PROP_VALUE

void CombatUnit::GetEnemy(ObjIfVec& list) const
{
	std::vector<CombatUnit*> enemies;
    if (IsGolem())
    {
	    const CombatNpc* npc = dynamic_cast<const CombatNpc*>(this);
	    combat_->GetEnemy(npc->GetOwner()->GetXID(), enemies);
    }
    else
    {
	    combat_->GetEnemy(xid_, enemies);
    }

	list.clear();
	list.resize(MAX_COMBAT_UNIT_NUM);
	for (size_t i = 0; i < enemies.size(); ++ i)
	{
		if (!enemies[i]) continue;
		CombatUnit* unit = enemies[i];
		list[unit->GetPos()] = unit->GetObjIf();
	}
}

void CombatUnit::GetTeamMate(ObjIfVec& list) const
{
	std::vector<CombatUnit*> mates;
    if (IsGolem())
    {
	    const CombatNpc* npc = dynamic_cast<const CombatNpc*>(this);
        combat_->GetTeamMate(npc->GetOwner()->GetXID(), mates);
    }
    else
    {
	    combat_->GetTeamMate(xid_, mates);
    }

	list.clear();
	list.resize(MAX_COMBAT_UNIT_NUM);
	for (size_t i = 0; i < mates.size(); ++ i)
	{
		if (!mates[i]) continue;
		CombatUnit* unit = mates[i];
		list[unit->GetPos()] = unit->GetObjIf();
	}
}

void CombatUnit::GetGolem(ObjIfVec& list) const
{
	std::vector<CombatUnit*> golems;
    combat_->GetGolem(golems);
	list.clear();
	for (size_t i = 0; i < golems.size(); ++ i)
	{
		if (!golems[i]) continue;
		CombatUnit* unit = golems[i];
        list.push_back(unit->GetObjIf());
	}
}

int CombatUnit::GetEnemiesAlive() const
{
    if (IsGolem())
    {
	    const CombatNpc* npc = dynamic_cast<const CombatNpc*>(this);
	    return combat_->GetEnemiesAlive(npc->GetOwner()->GetXID());
    }
    else
    {
	    return combat_->GetEnemiesAlive(xid_);
    }
}

int CombatUnit::GetTeammatesAlive() const
{
    if (IsGolem())
    {
	    const CombatNpc* npc = dynamic_cast<const CombatNpc*>(this);
	    return combat_->GetTeammatesAlive(npc->GetOwner()->GetXID());
    }
    else
    {
	    return combat_->GetTeammatesAlive(xid_);
    }
}

bool CombatUnit::IsTeammate(UnitID unit_id) const
{
	return combat_->IsTeammate(obj_if_->GetOwner()->GetId(), unit_id);
}

bool CombatUnit::IsEnemy(UnitID unit_id) const
{
	return combat_->IsEnemy(obj_if_->GetOwner()->GetId(), unit_id);
}

void CombatUnit::SaveBuffMessage(const skill::BuffDamageVec& damages)
{
	if (damages.empty())
		return;

	buff_damages_.insert(buff_damages_.end(), damages.begin(), damages.end());

	char buf[1024];
	memset(&buf, 0, sizeof(buf) / sizeof(char));
	sprintf((char*)(buf), "----------------------产生BUFF消息: ");
	for (size_t i = 0; i < damages.size(); ++ i)
	{
		skill::BuffDamage dmg = damages[i];
		sprintf((char*)(buf+strlen(buf)), "(攻击者=%ld, 被攻击者=%ld, 效果ID=%d), ", dmg.attacker, dmg.defender, dmg.effectid);
	}

	__PRINTF("%s", buf);
}

void CombatUnit::UpdateAndSendProp()
{
	//刷新属性
    RefreshExtendProp();

	//同步基础属性
	SendBasicProp();

	//同步扩展属性
	SendExtendProp();

	//广播易变属性
	SendVolatileProp();

    // 广播当前魔偶的易变属性
    SendCurGolemProp();

    // 同步魔偶属性
    SendOtherGolemProp();

	ClrRefreshBasicProp();
	ClrRefreshExtPropMask();
	ClrRefreshVolatileProp();
    ClrRefreshCurGolem();
    ClrRefreshOtherGolem();
}

void CombatUnit::SendExtendProp() const
{
	if (GetRefreshExtPropMask() && IsPlayer())
	{
	    std::vector<int> props;

        //保存变化的属性
        int32_t mask = refresh_extprop_mask_;
        int32_t index = 0;
        while (mask)
        {
            while (index < PROP_INDEX_HIGHEST)
            {
                if (mask & (1 << index))
                    break;
                else
                    ++ index;
            }

            //这里必须检查index值
            //否则将同步无效值给客户端
            if (index >= PROP_INDEX_HIGHEST)
            {
                break;
            }

            props.push_back(cur_prop_[index]);

            //消去最低位1
            mask &= mask-1;
        };

        //同步给玩家本人
        G2C::CombatPlayerExtProp packet;
		packet.prop_mask = refresh_extprop_mask_;
		packet.props = props;
		SendPacket(packet);
	}
}

void CombatUnit::SendVolatileProp() const
{
	if (TestRefreshVolatileProp())
	{
		//广播基础属性
		G2C::CombatUnitVolatileProp packet;
		packet.unit_id = GetID();
		packet.hp      = GetHP();
		packet.max_hp  = GetMaxHP();
		//BroadCastCMD(packet, xid_.id);
        // 因为排除了自己，被动技能增加的最大血量自己不会导致最大血量的同步
		BroadCastCMD(packet);
	}
}

void CombatUnit::SendBasicProp() const
{
	if (TestRefreshBasicProp() && IsPlayer())
	{
		//同步基础属性
        G2C::CombatPlayerBaseProp packet;
        packet.unit_id = GetID();
        packet.hp = GetHP();
        packet.mp = GetMP();
        packet.ep = GetEP();
        SendPacket(packet);
	}
}

void CombatUnit::SendCurGolemProp() const
{
	if (TestRefreshCurGolem())
	{
        OnSendCurGolemProp();
	}
}

void CombatUnit::SendOtherGolemProp() const
{
	if (TestRefreshOtherGolem() && IsPlayer())
	{
        OnSendOtherGolemProp();
	}
}

void CombatUnit::SendBuffResult(const skill::BuffDamageVec& buff_damages) const
{
    if (buff_damages.size() <= 0)
        return;

	G2C::CombatBuffResult packet;
	packet.combat_id = combat_->GetID();
	for (size_t i = 0; i < buff_damages.size(); ++ i)
	{
		const skill::BuffDamage& damage = buff_damages[i];

		G2C::buff_entry buff;
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

		packet.buff_list.push_back(buff);
	}

	BroadCastCMD(packet);
}

void CombatUnit::Speak(int id_talk, int msec)
{
	G2C::CombatUnitSpeak packet;
	packet.unit_id   = xid_.id;
	packet.talk_id   = id_talk;
	packet.talk_time = msec;
	BroadCastCMD(packet);
}

void CombatUnit::Consume(SkillID skillid)
{
    skill_->Consume(skillid);
}

int CombatUnit::GetGolem(TemplID golem_id)
{
	for (size_t i = 0; i < golem_list_.size(); ++ i)
		if (golem_list_[i]->GetNpcID() == golem_id)
			return i;
	return -1;
}

bool CombatUnit::SummonGolem(TemplID golem_id)
{
	if (!s_golem_man.IsGolemValid(golem_id))
	{
		assert(false);
		return false;
	}

	if (cur_golem_ != NULL && cur_golem_->GetNpcID() == golem_id)
	{
		return false;
	}

	int index = GetGolem(golem_id);
	if (index >= 0)
	{
		CombatNpc* & __cur_golem = cur_golem_;
		CombatNpc* & __new_golem = golem_list_[index];
		if (!__new_golem->CanSummoned())
		{
			assert(false);
			return false;
		}

		__PRINTF("玩家 %d 切换魔偶(%d ==>> %d)", GetID(), __cur_golem->GetNpcID(), __new_golem->GetNpcID());

		__cur_golem->OnDeActived();
		__new_golem->OnActived();
        __new_golem->CombatStart();
		std::swap(__cur_golem, __new_golem);
        SetRefreshCurGolem();
		return true;
	}

	//创建新魔偶
	PropsVec props;
	std::vector<SkillID> skills;
    std::string model;
	s_golem_man.GetGolemProps(golem_id, props);
	s_golem_man.GetGolemSkill(golem_id, skills);
	s_golem_man.GetGolemModel(golem_id, model);

	CombatNpc* golem = s_pCombatMan->AllocNPC();
	if (!golem)
	{
		return false;
	}

	golem->SetPos(GetPos());
	golem->SetNpcID(golem_id);
	golem->SetNpcType(CombatNpc::TYPE_NPC_GOLEM);
    golem->SetModel(model);
	golem->Initialize();
	golem->SetCombat(combat_);
	golem->SetOwner(this);
	golem->SetParty(party_);

	//加入对象池
    combat_->AddGolem(golem);
	s_pCombatMan->InsertNPCToMan(golem);

	//解锁对象
	golem->Unlock();

	__PRINTF("玩家 %d 召唤魔偶(%d)", GetID(), golem_id);

	if (!cur_golem_)
	{
		//首次召唤魔偶
		cur_golem_ = golem;
	}
	else
	{
		//召唤新魔偶
		cur_golem_->OnDeActived();
		golem_list_.push_back(cur_golem_);

		cur_golem_ = golem;
	}
    //cur_golem_->OnActived();
    cur_golem_->CombatStart();

	return true;
}

bool CombatUnit::PowerGolem(int32_t power)
{
    if (cur_golem_ != NULL)
    {
        cur_golem_->IncProp(PROP_INDEX_MP, power);
        SetRefreshCurGolem();
    }
	for (size_t i = 0; i < golem_list_.size(); ++ i)
    {
        golem_list_[i]->IncProp(PROP_INDEX_MP, power);
    }
    if (golem_list_.size() != 0)
    {
        SetRefreshOtherGolem();
    }
	return true;
}

int32_t CombatUnit::GetGolemProp(int32_t golem_id, size_t index)
{
    if (cur_golem_ != NULL && cur_golem_->GetNpcID() == golem_id)
    {
        return cur_golem_->GetProp(index);
    }
	for (size_t i = 0; i < golem_list_.size(); ++ i)
    {
		if (golem_list_[i]->GetNpcID() == golem_id)
        {
            return golem_list_[i]->GetProp(index);
        }
    }
    return 0;
}

int32_t CombatUnit::GetGolemDeActivePower(TemplID golem_id)
{
	for (size_t i = 0; i < golem_list_.size(); ++ i)
    {
		if (golem_list_[i]->GetNpcID() == golem_id)
        {
            return golem_list_[i]->GetDeActivePower();
        }
    }
    return 0;
}

UnitID CombatUnit::GetCurGolemID() const
{
    return cur_golem_ == NULL ? 0 : cur_golem_->GetID();
}

void CombatUnit::Trace() const
{
	__PRINTF("----------------------战斗单位信息----------------------");

	const char* unit_name = NULL;
	if (IsPlayer()) unit_name = "玩家";
	else if (IsMob()) unit_name = "怪物";
	else if (IsPet()) unit_name = "宠物";
	else if (IsGolem()) unit_name = "魔偶";
	else if (IsTeamNpc()) unit_name = "组队NPC";
	else if (IsBoss()) unit_name = "世界BOSS";
	else assert(false);

	char buf[1024];
	memset(&buf, 0, sizeof(buf) / sizeof(char));
	sprintf((char*)(buf), "%s", "类型: ");
	sprintf((char*)(buf+strlen(buf)), "%s", unit_name);
	__PRINTF("%s, ", buf);

	__PRINTF("XID(%d, %d), POS(%d), PARTY(%d), STATUS(%d), 技能(%d), ", xid_.type, xid_.id, pos_, party_, unit_state_.GetStatus(), default_skill_);
	__PRINTF("HP(%d), CON1(%d), CON2(%d), MODEL(%s)", basic_prop_.hp, basic_prop_.mp, basic_prop_.ep, model_.c_str());
    __PRINTF("PROP(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", cur_prop_[0], cur_prop_[1], cur_prop_[2], cur_prop_[3],
                                                                    cur_prop_[4], cur_prop_[5], cur_prop_[6], cur_prop_[7],
                                                                    cur_prop_[8], cur_prop_[9], cur_prop_[10], cur_prop_[11]);
}

void CombatUnit::dump() const
{
    //combat_->Trace();
}

};
