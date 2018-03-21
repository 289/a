#include "combat_npc.h"
#include "combat.h"
#include "combat_def.h"
#include "combat_player.h"
#include "combat_types.h"
#include "mob_manager.h"
#include "global_drop.h"
#include "ai_npc.h"

#include "data_templ/templ_manager.h"
#include "data_templ/pet_templ.h"
#include "client_proto/G2C_proto.h"
#include "shared/security/randomgen.h"
#include "skill/include/skill_info.h"

namespace combat
{
/********************************CombatNpc*************************/
/********************************CombatNpc*************************/
/********************************CombatNpc*************************/
/********************************CombatNpc*************************/

CombatNpc::CombatNpc():
	CombatUnit(OBJ_TYPE_NPC),
	npc_id_(0),
	npc_type_(0),
	npc_ai_(NULL),
	npc_imp_(NULL)
{
}

CombatNpc::~CombatNpc()
{
}

bool CombatNpc::IsPet() const
{
	return npc_type_ == TYPE_NPC_PET;
}

bool CombatNpc::IsGolem() const
{
	return npc_type_ == TYPE_NPC_GOLEM;
}

bool CombatNpc::IsMob() const
{
	return npc_type_ == TYPE_NPC_MOB;
}

bool CombatNpc::IsTeamNpc() const
{
	return npc_type_ == TYPE_NPC_TEAMNPC;
}

bool CombatNpc::IsBoss() const
{
	return npc_type_ == TYPE_NPC_BOSS;
}

TemplID CombatNpc::GetNpcID() const
{
	return npc_id_;
}

void CombatNpc::SetNpcID(TemplID npc_id)
{
	assert(npc_id >= 0);
	npc_id_ = npc_id;
}

char CombatNpc::GetNpcType() const
{
	return npc_type_;
}

void CombatNpc::SetNpcType(char type)
{
	npc_type_ = type;
}

void CombatNpc::SetOwner(CombatUnit* owner)
{
	npc_imp_->SetOwner(owner);
}

CombatUnit* CombatNpc::GetOwner() const
{
	return npc_imp_->GetOwner();
}

void CombatNpc::Clear()
{
	CombatUnit::Clear();

	npc_id_   = 0;
	npc_type_ = 0;

	if (npc_imp_)
	{
		delete npc_imp_;
		npc_imp_ = NULL;
	}

	if (npc_ai_)
	{
		delete npc_ai_;
		npc_ai_ = NULL;
	}
}

void CombatNpc::OnInit()
{
	switch (npc_type_)
	{
		case TYPE_NPC_PET:
			{
				npc_imp_ = new PetImp(this);
				unit_state_.Init(UT_PET);
			}
			break;
		case TYPE_NPC_GOLEM:
			{
				npc_imp_ = new GolemImp(this);
				unit_state_.Init(UT_GOLEM);
			}
			break;
		case TYPE_NPC_MOB:
		case TYPE_NPC_TEAMNPC:
			{
				npc_ai_  = new NpcAI(this);
				npc_imp_ = new MobImp(this);
				unit_state_.Init(UT_MOB);
			}
			break;
		case TYPE_NPC_BOSS:
			{
				npc_ai_  = new NpcAI(this);
				npc_imp_ = new BossImp(this);
				unit_state_.Init(UT_BOSS);
			}
			break;
		default:
			{
				assert(false);
			}
			break;
	};

	npc_imp_->OnInit();
}

void CombatNpc::OnDying()
{
	npc_imp_->OnDying();
}

void CombatNpc::OnDeath()
{
	npc_imp_->OnDeath();
}

void CombatNpc::OnAttack()
{
	npc_imp_->OnAttack();
}

void CombatNpc::OnAttackEnd()
{
	npc_imp_->OnAttackEnd();
}

void CombatNpc::OnTransformWaitEnd()
{
    npc_imp_->OnTransformWaitEnd();
}

void CombatNpc::OnEscapeWaitEnd()
{
    npc_imp_->OnEscapeWaitEnd();
}

void CombatNpc::OnUpdateAttackState(int32_t action_time)
{
    if (IsGolem())
    {
	    npc_imp_->OnUpdateAttackState(action_time);
    }
    else
    {
        UpdateState(EVENT_ACTION, action_time);
    }
}

void CombatNpc::OnRoundStart()
{
	if (npc_ai_)
	{
		npc_ai_->OnRoundStart();
	}

	npc_imp_->OnRoundStart();
}

void CombatNpc::OnRoundEnd()
{
	if (npc_ai_)
	{
		npc_ai_->OnRoundEnd();
	}

	npc_imp_->OnRoundEnd();
}

void CombatNpc::OnDamage(int32_t dmg, UnitID attacker)
{
	if (npc_ai_)
	{
		npc_ai_->OnDamage();
	}
	npc_imp_->OnDamage(dmg, attacker);
}

void CombatNpc::OnTeammateDead()
{
	if (npc_ai_)
	{
		npc_ai_->OnTeammateDead();
	}
}

void CombatNpc::OnHeartBeat()
{
	npc_imp_->OnHeartBeat();
}

void CombatNpc::DoHeal(int life, UnitID healer)
{
	npc_imp_->DoHeal(life, healer);
}

void CombatNpc::DoDamage(int damage, UnitID attacker)
{
	npc_imp_->DoDamage(damage, attacker);
}

void CombatNpc::IncProp(size_t index, int value)
{
	npc_imp_->IncProp(index, value);
}

void CombatNpc::DecProp(size_t index, int value)
{
	npc_imp_->DecProp(index, value);
}

void CombatNpc::IncPropScale(size_t index, int value)
{
	npc_imp_->IncPropScale(index, value);
}

void CombatNpc::DecPropScale(size_t index, int value)
{
	npc_imp_->DecPropScale(index, value);
}

void CombatNpc::SetSkill(SkillID skill_id)
{
	npc_imp_->SetSkill(skill_id);
}

void CombatNpc::CastInstantSkill(SkillID skill_id)
{
	npc_imp_->CastInstantSkill(skill_id);
}

void CombatNpc::TriggerTransform(TemplID new_mob_tid)
{
    npc_imp_->TriggerTransform(new_mob_tid);
}

void CombatNpc::Transform(TemplID new_mob_tid)
{
    npc_imp_->Transform(new_mob_tid);
}

void CombatNpc::TriggerEscape(int32_t result)
{
    npc_imp_->TriggerEscape(result);
}

void CombatNpc::Escape()
{
    npc_imp_->Escape();
}

SkillID CombatNpc::GetSkill() const
{
	SkillID __skill = npc_imp_->GetSkill();
	if (__skill > 0)
	{
		return __skill;
	}
	return GetDefaultSkill();
}

int32_t CombatNpc::GetProp(size_t index) const
{
	return npc_imp_->GetProp(index);
}

void CombatNpc::GetEnemy(ObjIfVec& list) const
{
	npc_imp_->GetEnemy(list);
}

void CombatNpc::GetTeamMate(ObjIfVec& list) const
{
	npc_imp_->GetTeamMate(list);
}

void CombatNpc::SetPetBLevel(int pet_blevel)
{
	npc_imp_->SetPetBLevel(pet_blevel);
}

void CombatNpc::SetPetCombatPos(int pet_combat_pos)
{
	npc_imp_->SetPetCombatPos(pet_combat_pos);
}

int CombatNpc::GetPetCombatPos() const
{
	return npc_imp_->GetPetCombatPos();
}

int32_t CombatNpc::GetPowerConsume() const
{
	return npc_imp_->GetPowerConsume();
}

void CombatNpc::RestorePower()
{
	npc_imp_->RestorePower();
}

void CombatNpc::ConsumePower()
{
    Consume(skill_damages_[0].skillid);
}

int CombatNpc::GetDeActivePower() const
{
    return npc_imp_->GetDeActivePower();
}

void CombatNpc::OnActived()
{
	npc_imp_->OnActived();
}

void CombatNpc::OnDeActived()
{
	npc_imp_->OnDeActived();
}

bool CombatNpc::CanSummoned() const
{
	return unit_state_.OptionPolicy(OPT_SUMMONED);
}

bool CombatNpc::CanTransform() const
{
	return unit_state_.OptionPolicy(OPT_TRANSFORM);
}

bool CombatNpc::CanEscape() const
{
	return unit_state_.OptionPolicy(OPT_ESCAPE);
}

void CombatNpc::SaveForClient(G2C::CombatMobInfo& info) const
{
	npc_imp_->SaveForClient(info);
}

void CombatNpc::SyncBossHP(int32_t new_hp)
{
	npc_imp_->SyncBossHP(new_hp);
}

void CombatNpc::Trace() const
{
	CombatUnit::Trace();

	if (IsPet())
	{
	}
	else if (IsGolem())
	{
		__PRINTF("魔偶模板ID：%d, Npc类型：%d", npc_id_, npc_type_);
	}
	else if (IsMob())
	{
		__PRINTF("怪物模板ID：%d, Npc类型：%d", npc_id_, npc_type_);
	}
	else if (IsTeamNpc())
	{
		__PRINTF("组队NPCID：%d, Npc类型：%d", npc_id_, npc_type_);
	}
	else if (IsBoss())
	{
		__PRINTF("世界BOSS：%d, Npc类型：%d", npc_id_, npc_type_);
	}

	npc_imp_->Trace();
}

/********************************AnimalImp*************************/
/********************************AnimalImp*************************/
/********************************AnimalImp*************************/
/********************************AnimalImp*************************/
void AnimalImp::DoHeal(int life, UnitID healer)
{
	if (life <= 0) return;
	owner_->DoHeal(life, healer);
}

void AnimalImp::DoDamage(int damage, UnitID attacker)
{
	if (damage <= 0) return;
	owner_->DoDamage(damage, attacker);
}

void AnimalImp::IncProp(size_t index, int value)
{
	if (value <= 0) return;
	owner_->IncProp(index, value);
}

void AnimalImp::DecProp(size_t index, int value)
{
	if (value <= 0) return;
	owner_->DecProp(index, value);
}

void AnimalImp::IncPropScale(size_t index, int value)
{
	if (value <= 0) return;
	owner_->IncPropScale(index, value);
}

void AnimalImp::DecPropScale(size_t index, int value)
{
	if (value <= 0) return;
	owner_->DecPropScale(index, value);
}

void AnimalImp::GetEnemy(ObjIfVec& list) const
{
	owner_->GetEnemy(list);
}

void AnimalImp::GetTeamMate(ObjIfVec& list) const
{
	owner_->GetTeamMate(list);
    // 不需要ljh
	//list.push_back(parent_->GetObjIf());
}

void NpcImp::SetOwner(CombatUnit* owner)
{
    assert(owner);
	owner_ = owner;
}

CombatUnit* NpcImp::GetOwner() const
{
	return owner_;
}

/********************************PetImp*************************/
/********************************PetImp*************************/
/********************************PetImp*************************/
/********************************PetImp*************************/
void PetImp::OnInit()
{
	//宠物初始化需要等到血脉等级设置后
}

void PetImp::OnRoundStart()
{
	//宠物受伤害时由玩家来承担，宠物身上可能掉血BUFF，所以可能导致主人死亡
	//如果主人死亡，这里需要重置其状态时间
	/*if (owner_->IsZombie())
	{
		int32_t timeout = MAX_BUFF_PLAY_TIME;
		assert(owner_->GetStateTimeout() == -1);
		owner_->SetStateTimeout(timeout);

		__PRINTF("玩家 %d 承担宠物(%d)回合开始BUFF伤害而死亡!!! 重置状态STATUS_ZOMBIE时间成功. timeout:%d",
				owner_->GetID(), parent_->GetID(), timeout);
	}*/
}

void PetImp::OnRoundEnd()
{
	//do nothing
}

void PetImp::OnAttack()
{
	//宠物攻击时调用
	//消耗宠物能量
    CombatPlayer* player = dynamic_cast<CombatPlayer*>(owner_);
	player->GetPetMan()->ConsumePower(consume_on_attack_);

	//设置宠物冷却
	player->SetCoolDown(COOL_DOWN_INDEX_PET_ATTACK, player->GetPetMan()->GetAttackCDTime());
}

void PetImp::OnAttackEnd()
{
	//宠物受伤害时由玩家来承担，所以宠物受击可能导致主人死亡
	//如果主人死亡，这里需要重置其状态时间
	/*if (owner_->IsZombie())
	{
		assert(owner_->GetStateTimeout() == -1);

		skill::PauseTime time_map;
		skill::GetPauseTime(parent_->GetModel(), parent_->GetSkillDamages(), time_map);
		skill::PauseTime::iterator it = time_map.find(parent_->GetID());
		assert(it != time_map.end());
		owner_->SetStateTimeout(it->second);

		__PRINTF("玩家 %d 承担宠物(%d)伤害而死亡!!! 重置状态STATUS_ZOMBIE时间成功. timeout:%d",
				owner_->GetID(), parent_->GetID(), it->second);
	}*/
}

void PetImp::SetPetBLevel(int pet_blevel)
{
	pet_blevel_ = pet_blevel;

	TemplID pet_id = parent_->GetNpcID();
	const dataTempl::PetTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::PetTempl>(pet_id);
	assert(pTpl && "非法宠物ID");

	//计算宠物属性
	std::vector<int32_t> props;
	for (size_t i = 0; i < pTpl->properties.size(); ++ i)
	{
		int32_t init_prop = pTpl->properties[i].init_prop;
		int32_t inc_value = pTpl->properties[i].lvlup_inc;
		props.push_back(init_prop + (inc_value * parent_->GetLevel() * pet_blevel_));
	}

	//设置宠物属性
	parent_->SetBaseProp(props);

	SkillID __skillid = 0;
	int32_t __consume = 0;

	//获取宠物的技能和能量消耗
	for (size_t i = 0; i < pTpl->ranks.size(); ++ i)
	{
		if (pTpl->ranks[i].blevel_limit == 0)
		{
			break;
		}

		if (pet_blevel_ > pTpl->ranks[i].blevel_limit)
		{
			__skillid = pTpl->ranks[i].skill_id;
			__consume = pTpl->ranks[i].skill_consume;
			continue;
		}
		else if (pet_blevel_ == pTpl->ranks[i].blevel_limit)
		{
			__skillid = pTpl->ranks[i].skill_id;
			__consume = pTpl->ranks[i].skill_consume;
			break;
		}
		else if (pet_blevel_ < pTpl->ranks[i].blevel_limit)
		{
			break;
		}
	}

	//设置宠物默认技能
	parent_->SetDefaultSkill(__skillid);

	//设置能量消耗
	consume_on_attack_ = __consume;
}

void PetImp::SetPetCombatPos(int pet_combat_pos)
{
	pet_combat_pos_ = pet_combat_pos;
}

int PetImp::GetPetCombatPos() const
{
	return pet_combat_pos_;
}

int32_t PetImp::GetPowerConsume() const
{
	return consume_on_attack_;
}

void PetImp::Trace() const
{
	__PRINTF("能量消耗速度: %d", consume_on_attack_);
}

/********************************GolemImp*************************/
/********************************GolemImp*************************/
/********************************GolemImp*************************/
/********************************GolemImp*************************/
void GolemImp::OnInit()
{
	TemplID golem_id = parent_->GetNpcID();

	PropsVec props;
	s_golem_man.GetGolemProps(golem_id, props);
	parent_->SetBaseProp(props);
    parent_->SetBasicProp(0, props[PROP_INDEX_MAX_MP], 0);

	//TODO
	//基础属性未设置

	SkillVec skills;
	s_golem_man.GetGolemSkill(golem_id, skills);
    // 至少需要有一个被动技能，一个普通技能
    ASSERT(skills.size() >= 2);
	for (size_t i = 0; i < skills.size(); ++ i)
	{
		parent_->AddSkill(skills[i]);
		skill_group_.push_back(skills[i]);
	}
    appear_ = true;
    cur_skill_idx_ = skill_group_.size() - 2;
}

void GolemImp::OnRoundStart()
{
}

void GolemImp::OnRoundEnd()
{
	//do nothing
    cur_skill_idx_ = shared::net::RandomGen::RandUniform(0, skill_group_.size() - 2);
}

void GolemImp::OnUpdateAttackState(int32_t action_time)
{
    CombatUnit* owner = GetOwner();
    owner->UpdateState(EVENT_ACTION, action_time);
}

SkillID GolemImp::GetSkill() const
{
    skill::SkillWrapper* wrapper = parent_->GetSkillWrapper();
    int32_t skill_id = skill_group_[cur_skill_idx_];
    if (wrapper->CanCast(skill_id) == skill::ERR_SUCCESS)
    {
        return skill_id;
    }
    skill_id = skill_group_[0];
    if (wrapper->CanCast(skill_id) == skill::ERR_SUCCESS)
    {
        return skill_id;
    }
    return 0;
}

void GolemImp::IncProp(size_t index, int value)
{
    if (value <= 0) return;
    parent_->CombatUnit::IncProp(index, value);
}

void GolemImp::DecProp(size_t index, int value)
{
    if (value <= 0) return;
    parent_->CombatUnit::DecProp(index, value);
}

int32_t GolemImp::GetProp(size_t index) const
{
    int32_t owner_prop_value = 0;
	int32_t golem_prop_value = parent_->CombatUnit::GetProp(index);
    if (index != PROP_INDEX_MAX_MP && index != PROP_INDEX_MP)
    {
        owner_prop_value = owner_->GetProp(index);
    }
	return golem_prop_value + owner_prop_value;
}

void GolemImp::OnAttackEnd()
{
    if (!appear_)
    {
        parent_->ConsumePower();
        owner_->SetRefreshCurGolem();
    }
    appear_ = false;
}

void GolemImp::RestorePower()
{
	int32_t power_gen = 0;
	power_gen += parent_->GetProp(PROP_INDEX_POWER_GEN_SPEED);
	power_gen += owner_->GetProp(PROP_INDEX_POWER_GEN_SPEED);
	parent_->IncProp(PROP_INDEX_MP, power_gen);
}

void GolemImp::OnActived()
{
    appear_ = true;
	cur_skill_idx_ = skill_group_.size() - 2;
	parent_->UpdateState(EVENT_ACTIVED);
}

void GolemImp::OnDeActived()
{
	cur_skill_idx_ = 0;
    deactive_power_ = parent_->GetProp(PROP_INDEX_MP);

    /*skill::BuffWrapper* wrapper = parent_->GetBuffWrapper();
    skill::SkillDamageVec skilldmg_vec;
	skill::BuffDamageVec damages;
	wrapper->Dying(skilldmg_vec, damages);

	parent_->SendBuffResult(damages);*/

	parent_->UpdateState(EVENT_DEACTIVED);
}

int GolemImp::GetDeActivePower() const
{
    return deactive_power_;
}

void GolemImp::Trace() const
{
	char buf[1024];
	memset(&buf, 0, sizeof(buf) / sizeof(char));
	sprintf((char*)(&buf),"魔偶基础技能：");
	for (size_t i = 0; i < skill_group_.size(); ++ i)
		sprintf((char*)(&buf), "%d,", skill_group_[i]);
	__PRINTF("%s", buf);
}

/********************************MobImp*************************/
/********************************MobImp*************************/
/********************************MobImp*************************/
/********************************MobImp*************************/
void MobImp::OnInit()
{
	TemplID mob_id = parent_->GetNpcID();
	SkillID normal_attack_id = s_mob_man.GetMobNormalAtkID(mob_id);
	if (normal_attack_id > 0)
	{
		parent_->SetDefaultSkill(normal_attack_id);
	}

	MobManager::SkillGroup skills;
	s_mob_man.GetMobSkillGroup(mob_id, skills);
	for (size_t i = 0; i < skills.size(); ++ i)
	{
		parent_->AddSkill(skills[i].skill);

		SkillEntry entry;
		entry.skill_id = skills[i].skill;
		entry.cast_probability = skills[i].prob;
		if (i > 0)
		{
			entry.cast_probability += skill_group_[i-1].cast_probability;
		}
		skill_group_.push_back(entry);
	}

	PropsVec props;
	s_mob_man.GetMobProps(mob_id, props);
	parent_->SetBaseProp(props);
	parent_->SetBasicProp(props[PROP_INDEX_MAX_HP], props[PROP_INDEX_MAX_MP], props[PROP_INDEX_MAX_EP]);

	parent_->SetATBOwner();

	parent_->SetATBTime(parent_->GetATBTime());

    //设置怪物模型资源路径
    parent_->SetModel(s_mob_man.GetMobModel(mob_id));
}

void MobImp::OnDamage(int32_t dmg, UnitID attacker)
{
}

void MobImp::OnDying()
{
	parent_->Die();
}

void MobImp::OnDeath()
{
	Combat* combat = parent_->GetCombat();

	if (parent_->IsMob() || parent_->IsBoss())
	{
		/**
		 * 怪物死亡掉落分为五种：
		 * 1) 经验(战斗结束发放);
		 * 2) 金钱(战斗结束发放);
		 * 3) 普通掉落(怪物死亡掉落);
		 * 4) 特殊掉落(战斗结束奖励);
		 * 5) 全局掉落(额外奖励);
		 * 6) 怪物死亡时，只会执行3和5，掉落1,2,4都在战斗结束时执行
		 */

		std::vector<ItemEntry> items_drop;

		UnitID killer  = parent_->GetKiller();
		TemplID mob_id = parent_->GetNpcID();

		//执行怪物死亡掉落
		std::vector<ItemEntry> normal_drop;
		s_mob_man.GenerateDropItem(mob_id, normal_drop);

		//执行怪物死亡的全局掉落
		int32_t exp_drop = 0;
		int32_t money_drop = 0;
		std::vector<ItemEntry> global_drop;
		s_global_drop_man.GenerateDrop(combat->GetMapID(), mob_id, exp_drop, money_drop, global_drop);

		//计算怪我死亡掉落
		items_drop.insert(items_drop.end(), normal_drop.begin(), normal_drop.end());
		items_drop.insert(items_drop.end(), global_drop.begin(), global_drop.end());

		//把掉落和杀跌个数同步到战场
		CombatPVE* pve_combat = dynamic_cast<CombatPVE*>(combat);
		pve_combat->OnMobKilled(parent_->GetNpcID(), money_drop, exp_drop, items_drop, killer);

		//广播怪物死亡
		G2C::CombatMobDead packet;
		packet.mob_unit_id  = parent_->GetID();
		packet.killer_unit_id = parent_->GetKiller();
		for (size_t i = 0; i < items_drop.size(); ++ i)
		{
			G2C::ItemEntry item;
			item.item_id = items_drop[i].item_id;
			item.item_count = items_drop[i].item_count;
			packet.items_drop.push_back(item);
		}
		combat->BroadCastCMD(packet);
	}
	else
	{
		//组队NPC死亡,
		//不执行怪物的掉落逻辑
		//广播组队NPC死亡
		G2C::CombatMobDead packet;
		packet.mob_unit_id  = parent_->GetID();
		combat->BroadCastCMD(packet);
	}

	__PRINTF("----------------------怪物(id=%d)死亡了!!!", parent_->GetID());
}

void MobImp::OnTransformWaitEnd()
{
    //怪物开始变身
    MSG msg;
    BuildMessage(msg, COMBAT_MSG_START_TRANSFORM, parent_->GetCombat()->GetXID(), parent_->GetXID(), new_mob_tid_, 0, 0);
    parent_->SendMSG(msg);

    new_mob_tid_ = 0;
}

void MobImp::OnEscapeWaitEnd()
{
    //怪物开始逃跑
    MSG msg;
    BuildMessage(msg, COMBAT_MSG_START_ESCAPE, parent_->GetCombat()->GetXID(), parent_->GetXID(), 0, 0, 0);
    parent_->SendMSG(msg);
}

void MobImp::OnRoundStart()
{
	//do nothing
}

void MobImp::OnRoundEnd()
{
	if (ai_skill_ > 0 && parent_->GetRoundCounter() >= ai_skill_rounder_)
	{
		ai_skill_ = 0;
	}
}

SkillID MobImp::GetSkill() const
{
    int32_t skillid = ai_skill_;
    skill::SkillWrapper* wrapper = parent_->GetSkillWrapper();
	if (skillid > 0 && wrapper->CanCast(skillid) == skill::ERR_SUCCESS)
		return skillid;

	int32_t rand_num = shared::net::RandomGen::RandUniform(1, 10000);
	for (size_t i = 0; i < skill_group_.size(); ++ i)
	{
		const SkillEntry& entry = skill_group_[i];
        skillid = entry.skill_id;
		if (rand_num <= entry.cast_probability)
		{
            return wrapper->CanCast(skillid) == skill::ERR_SUCCESS ? skillid : 0;
		}
	}

	return 0;
}

void MobImp::GetEnemy(ObjIfVec& list) const
{
	parent_->CombatUnit::GetEnemy(list);
}

void MobImp::GetTeamMate(ObjIfVec& list) const
{
	parent_->CombatUnit::GetTeamMate(list);
}

void MobImp::SetSkill(SkillID skillid)
{
	if (skillid <= 0)
	{
		return;
	}

	ai_skill_ = skillid;
	ai_skill_rounder_ = parent_->GetRoundCounter() + 1;
	parent_->AddSkill(ai_skill_);

	__PRINTF("战斗对象 %d 获得AI技能, skill_id: %d.", parent_->GetID(), skillid);
}

void MobImp::CastInstantSkill(SkillID skillid)
{
	SetSkill(skillid);
	parent_->RegisterInstantATB();
}

void MobImp::TriggerTransform(TemplID new_mob_tid)
{
    if (parent_->CanTransform())
    {
        //触发怪物变身事件
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_TRIGGER_TRANSFORM, parent_->GetCombat()->GetXID(), parent_->GetXID(), new_mob_tid, 0, 0);
        parent_->SendMSG(msg);

        parent_->UnRegisterATB();
        parent_->SetTransforming();
    }
    else
    {
        __PRINTF("战斗对象 %d 变身失败!!!", parent_->GetID());
    }
}

void MobImp::Transform(TemplID new_mob_tid)
{
    if (parent_->CanTransform())
    {
        int duration = -1;
        if (parent_->IsNormal())
        {
            duration = 100;
        }
        else
        {
            duration = parent_->GetStateTimeout() * MSEC_PER_TICK;
        }

        new_mob_tid_ = new_mob_tid;

        parent_->UpdateState(EVENT_TRANSFORM, duration);

        __PRINTF("战斗对象 %d 开始变身!!! new_mob_tid: %d", parent_->GetID(), new_mob_tid);
    }
    else
    {
        __PRINTF("战斗对象 %d 变身失败了，不应该啊!", parent_->GetID());
        assert(false);
    }
}

void MobImp::TriggerEscape(int32_t result)
{
    if (parent_->CanEscape())
    {
        //触发怪物逃跑事件
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_TRIGGER_ESCAPE, parent_->GetCombat()->GetXID(), parent_->GetXID(), result, 0, 0);
        parent_->SendMSG(msg);

        parent_->UnRegisterATB();
        parent_->SetEscaping();
    }
    else
    {
        __PRINTF("战斗对象 %d 逃跑失败!!!", parent_->GetID());
    }
}

void MobImp::Escape()
{
    if (parent_->CanEscape())
    {
        int duration = -1;
        if (parent_->IsNormal())
        {
            duration = 1000;
        }
        else
        {
            duration = parent_->GetStateTimeout() * MSEC_PER_TICK + 1000;
        }

        parent_->UpdateState(EVENT_ESCAPE, duration);

        __PRINTF("战斗对象 %d 开始逃跑!!!", parent_->GetID());
    }
    else
    {
        __PRINTF("战斗对象 %d 逃跑失败了，不应该啊!", parent_->GetID());
        assert(false);
    }
}

void MobImp::SaveForClient(G2C::CombatMobInfo& info) const
{
	info.unit_id = parent_->GetID();
	info.mob_tid = parent_->GetNpcID();
	info.mob_hp  = parent_->GetHP();
	info.mob_pos = parent_->GetPos();
	info.sneak_attacked = parent_->IsSneaked() ? 1:0;
}

void MobImp::Trace() const
{
}

/********************************BossImp*************************/
/********************************BossImp*************************/
/********************************BossImp*************************/
/********************************BossImp*************************/
void BossImp::OnInit()
{
	MobImp::OnInit();
}

void BossImp::OnDamage(int32_t damage, UnitID attacker)
{
	MobImp::OnDamage(damage, attacker);

	CombatPVE* combat = dynamic_cast<CombatPVE*>(parent_->GetCombat());

	combat_msg_boss_damaged content;
	content.combat_id = combat->GetID();
	content.boss_pos = parent_->GetPos();
	content.attacker = attacker;
	content.damage = damage;

	MSG msg;
	BuildMessage(msg, COMBAT_MSG_BOSS_DAMAGED, MAKE_XID(combat->GetWorldBossID()), parent_->GetXID(), 0, &content, sizeof(content));
	parent_->SendMSG(msg);
}

void BossImp::OnHeal(int32_t life)
{
	CombatPVE* combat = dynamic_cast<CombatPVE*>(parent_->GetCombat());

	combat_msg_boss_healed content;
	content.combat_id = combat->GetID();
	content.boss_pos = parent_->GetPos();
	content.life = life;

	MSG msg;
	BuildMessage(msg, COMBAT_MSG_BOSS_HEALED, MAKE_XID(combat->GetWorldBossID()), parent_->GetXID(), 0, &content, sizeof(content));
	parent_->SendMSG(msg);
}

void BossImp::OnDeath()
{
	MobImp::OnDeath();

	__PRINTF("----------------------BOSS(id=%d)死亡了!!!", parent_->GetID());
}

void BossImp::SyncBossHP(int32_t new_hp)
{
	int32_t old_hp = parent_->GetHP();
	parent_->SyncHP(new_hp);

	if (new_hp != old_hp)
	{
		G2C::CombatUnitVolatileProp packet;
		packet.unit_id = parent_->GetID();
		packet.hp      = parent_->GetHP();
		packet.max_hp  = parent_->GetMaxHP();
		parent_->BroadCastCMD(packet);
	}
}

void BossImp::Trace() const
{
	MobImp::Trace();
}

};

