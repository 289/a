#include <assert.h>
#include <string.h>
#include <algorithm>

#include "combat_player.h"
#include "combat.h"
#include "combat_npc.h"
#include "combat_msg.h"
#include "combat_obj_if.h"

#include "client_proto/G2C_proto.h"
#include "skill/include/skill_types.h"
#include "skill/include/cooldown_wrapper.h"


namespace combat
{

struct skill_finder
{
	SkillID skill_id;
	skill_finder(SkillID id): skill_id(id) {}
	bool operator () (const CombatPlayer::SkillTree& rhs) const
	{
		return rhs.skill_id == skill_id;
	}
};

struct sk_tree_finder
{
	TemplID sk_tree_id;
	sk_tree_finder(TemplID id): sk_tree_id(id) {}
	bool operator () (const CombatPlayer::SkillTree& rhs) const
	{
		return rhs.skill_tree_id == sk_tree_id;
	}
};

CombatPlayer::CombatPlayer()
	: CombatUnit(OBJ_TYPE_PLAYER),
	  master_id_(0),
	  cur_skill_(0),
	  attack_done_(false),
	  offline_(false),
	  gen_mp_flag_(false),
      dying_time_(0)
{
	combat_award_.exp     = 0;
	combat_award_.money   = 0;

	role_.roleid   = 0;
	role_.cls      = 0;
	role_.level    = 0;
	role_.gender   = -1;
	role_.weapon_id= 0;
}

CombatPlayer::~CombatPlayer()
{
	golem_list_.clear();
	mob_list_killed_.clear();
	player_list_killed_.clear();
	combat_award_.items.clear();
	cooldown_man_.Clear();
}

void CombatPlayer::OnInit()
{
	SetATBOwner();
	unit_state_.Init(UT_PLAYER);
}

int CombatPlayer::GetSkill() const
{
    if (cur_skill_ != 0 && skill_->CanCast(cur_skill_) == skill::ERR_SUCCESS)
    {
        return cur_skill_;
    }
    else
    {
        return default_skill_;
    }
}

bool CombatPlayer::Load(const player_data& data)
{
	//设置masterid
	SetMasterServerID(data.master_id);

	//设置玩家基本属性
	SetRoleInfo(data.roleid, data.name, data.cls, data.gender, data.level, data.weapon_id);
	SetBasicProp(data.hp, data.mp, data.ep);

	//设置玩家扩展属性
	SetBaseProp(data.props);

	//设置战斗站位
	SetPos(data.pos);

	//设置玩家普攻ID
	SetDefaultSkill(data.normal_atk_id);

    //设置资源模型路径
    SetModel(data.cls_model_src);

    //设置玩家濒死时间
    dying_time_ = data.dying_time;
    golem_action_ = false;

    // 加载被动技能
    addon_skills_ = data.addon_skills;
    SkillSet::const_iterator it = addon_skills_.begin();
    for (; it != addon_skills_.end(); ++it)
    {
		//注册技能到技能系统
        if (*it != 0)
        {
		    AddSkill(*it);
        }
    }

	//加载技能数据
	skill_tree_vec_.resize(data.skill_tree_vec.size());
	for (size_t i = 0; i < data.skill_tree_vec.size(); ++ i)
	{
		const skill_tree& skill          = data.skill_tree_vec[i];
		skill_tree_vec_[i].skill_tree_id = skill.skill_tree_id;
		skill_tree_vec_[i].skill_id      = skill.skill_id;
		skill_tree_vec_[i].level         = skill.level;
		skill_tree_vec_[i].tmp_level     = skill.tmp_level;
		skill_tree_vec_[i].max_level     = skill.max_level;

		//注册技能到技能系统
		AddSkill(skill_tree_vec_[i].skill_id);
	}

	return true;
}

bool CombatPlayer::LoadPetData(const player_data& data)
{
	//初始化宠物管理器
	pet_man_.SetPlayer(this);
	pet_man_.SetCombat(combat_);

	//设置战宠数据
	if (!pet_man_.Load(data.petdata))
		return false;

	return true;
}

void CombatPlayer::SaveForClient(G2C::CombatPlayerInfo& pinfo) const
{
	pinfo.unit_id  = xid_.id;
	pinfo.roleid   = role_.roleid;
	pinfo.rolename = role_.rolename;
	pinfo.state    = GetStatus();
	pinfo.gender   = role_.gender;
	pinfo.cls      = role_.cls;
	pinfo.level    = role_.level;
	pinfo.pos      = pos_;
	pinfo.hp       = GetHP();
	pinfo.mp       = GetMP();
	pinfo.max_hp   = GetMaxHP();
	pinfo.weapon   = role_.weapon_id;
	pinfo.sneak_attacked = IsSneaked() ? 1:0;
    pinfo.which_party = party_;

    SkillTreeVec::const_iterator sit = skill_tree_vec_.begin();
    for (; sit != skill_tree_vec_.end(); ++sit)
    {
        pinfo.skill_list.push_back(sit->skill_id);
    }

    SkillSet::const_iterator ait = addon_skills_.begin();
    for (; ait != addon_skills_.end(); ++ait)
    {
        pinfo.skill_list.push_back(*ait);
    }

	pet_man_.Save(pinfo.pet_list);
}

void CombatPlayer::Clear()
{
	role_.roleid    = 0;
	role_.cls       = 0;
	role_.level     = 0;
	role_.gender    = -1;
	role_.weapon_id = 0;
	role_.rolename.clear();

	cur_skill_ = 0;
	attack_done_ = false;
	master_id_ = 0;

	offline_    = false;
	gen_mp_flag_= false;
    dying_time_ = 0;

	pet_man_.Clear();
    addon_skills_.clear();
	skill_tree_vec_.clear();
	mob_list_killed_.clear();
	player_list_killed_.clear();
	combat_award_.Clear();
	cooldown_man_.Clear();
	CombatUnit::Clear();
}

bool CombatPlayer::CMDHandler_PlayerOnline(const PlayerCMD& cmd)
{
    if (!offline_)
    {
        return false;
    }

	offline_ = false;

	//广播玩家上线
	G2C::CombatPlayerState packet1;
	packet1.unit_id = GetID();
	packet1.state = G2C::COMBAT_PLAYER_STATE_ONLINE;
	BroadCastCMD(packet1);

	//同步最新属性给玩家
	G2C::CombatPlayerExtProp packet;
	packet.prop_mask = MAX_PROP_MASK;
	for (int i = 0; i < PROP_INDEX_HIGHEST; ++ i)
	{
		packet.props.push_back(cur_prop_[i]);
	}
	SendPacket(packet);

	//发送战场数据给玩家
    if (combat_->GetType() == OBJ_TYPE_COMBAT_PVE)
    {
        G2C::CombatPVEStart packet;
        combat_->BuildCombatData(packet);
        packet.new_combat = 0;
        SendPacket(packet);
    }
    else if (combat_->GetType() == OBJ_TYPE_COMBAT_PVP)
    {
        G2C::CombatPVPStart packet;
        packet.which_party = party_;
        combat_->BuildCombatData(packet);
        SendPacket(packet);
    }

	//同步当前身上的BUFF给玩家
    G2C::CombatBuffSync buff_packet;
    combat_->BuildBuffSyncData(buff_packet);
    SendPacket(buff_packet);

    // 同步当前选中的技能以及技能冷却
    G2C::CombatSkillSync skill_packet;
    skill_packet.cur_skillid = cur_skill_;
    skill::CooldownWrapper* cd_wrapper = GetCDWrapper();
    cd_wrapper->GetCooldownInfo(skill_packet.cooldown);
    SendPacket(skill_packet);

    // 同步魔偶信息
    if (cur_golem_ != NULL)
    {
        G2C::CombatUnitCurGolemProp packet;
        packet.unit_id = GetID();
        packet.golem_id = cur_golem_->GetNpcID();
        packet.mp = cur_golem_->GetProp(PROP_INDEX_MP);
        SendPacket(packet);

        OnSendOtherGolemProp();
    }

    return true;
}

bool CombatPlayer::CMDHandler_PlayerOffline(const PlayerCMD& cmd)
{
    if (offline_)
    {
        return false;
    }

	offline_ = true;

	//广播玩家离线
	G2C::CombatPlayerState packet;
	packet.unit_id = GetID();
	packet.state = G2C::COMBAT_PLAYER_STATE_OFFLINE;
	BroadCastCMD(packet, xid_.id);

    return true;
}

void CombatPlayer::OnPlayerJoin()
{
	Trace();

	RegisterATB();

	//广播有新玩家加入战斗
	G2C::CombatPlayerJoin packet;
	packet.combat_id = combat_->GetCombatID();
	SaveForClient(packet.player_info);
	BroadCastCMD(packet, xid_.id);

	//将战场数据同步给新加入的玩家
    if (combat_->GetType() == OBJ_TYPE_COMBAT_PVE)
    {
        G2C::CombatPVEStart packet;
        combat_->BuildCombatData(packet);
        packet.new_combat = 0;
        SendPacket(packet);
    }
    else if (combat_->GetType() == OBJ_TYPE_COMBAT_PVP)
    {
        G2C::CombatPVPStart packet;
        packet.which_party = party_;
        combat_->BuildCombatData(packet);
        SendPacket(packet);
    }

	//同步当前身上的BUFF给玩家
    G2C::CombatBuffSync buff_packet;
    combat_->BuildBuffSyncData(buff_packet);
    SendPacket(buff_packet);

	CombatStart();
}

void CombatPlayer::OnAttack()
{
	//攻战类角色在攻击时恢复能量值
	//恢复与否由技能控制
	if (gen_mp_flag_)
	{
		//计算攻击造成的总伤害
		int32_t damage_total = 0;
		for (size_t i = 0; i < skill_damages_.size(); ++ i)
		{
			damage_total += CalDamageValue(skill_damages_[i]);
		}

		PropPolicy::UpdatePower(this, CT_ATTACK, damage_total);

		gen_mp_flag_ = false;
	}
}

void CombatPlayer::OnAttackEnd()
{
    Consume(skill_damages_[0].skillid);
}

void CombatPlayer::OnDamage(int32_t damage, UnitID attacker)
{
	//防战类角色在受击时恢复能量值
	PropPolicy::UpdatePower(this, CT_DAMAGED, damage);
}

void CombatPlayer::OnDying()
{
	skill::BuffDamageVec damages;
	buff_man_->Dying(skill_damages_, damages);

    if (!skill_damages_.empty())
    {
        //combat_->BroadcastSkillResult(this, skill_damages_);
        skill_damages_.clear();
    }
	SendBuffResult(damages);

    G2C::CombatPlayerState packet;
    packet.unit_id = GetID();
    packet.state = G2C::COMBAT_PLAYER_STATE_DYING;
    BroadCastCMD(packet);

    __PRINTF("----------------------玩家濒死了, unit_id=%d", xid_.id);
}

void CombatPlayer::OnDeath()
{
	G2C::CombatPlayerState packet;
	packet.unit_id = GetID();
	packet.state   = G2C::COMBAT_PLAYER_STATE_DEAD;
	BroadCastCMD(packet);

	__PRINTF("----------------------玩家死亡了, unit_id=%d", xid_.id);
}

void CombatPlayer::OnRoundStart()
{
}

void CombatPlayer::OnRoundEnd()
{
	//法师和技师恢复能量值
    if (!golem_action_)
    {
	    PropPolicy::UpdatePower(this, CT_ROUND_END);
    }
}

void CombatPlayer::OnHeartBeat()
{
	if (IsDying() && basic_prop_.hp > 0)
	{
		Revive();
	}

	pet_man_.HeartBeat();
	cooldown_man_.HeartBeat();
}
/*
bool CombatPlayer::StartAttack()
{
	//魔偶共享玩家的ATB
	//只有玩家和魔偶的攻击都完成才删除玩家的ATB

    if (player_action_)
    {
        //玩家攻击
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
*/
void CombatPlayer::OnUpdateAttackState(int32_t action_time)
{
	UpdateState(EVENT_ACTION, action_time);
}

void CombatPlayer::DoDamage(int damage, UnitID attacker)
{
	if (damage <= 0)
        return;

	if (IsZombie() || IsDead())
        return;

	int32_t old_hp = basic_prop_.hp;
	basic_prop_.hp -= damage;
	if (basic_prop_.hp <= 0 && killer_ == 0)
	{
		SetKiller(attacker);
	}

	if (old_hp > 0 && basic_prop_.hp <= 0)
	{
	    if (GetTeammatesAlive() <= 1)
        {
		    UpdateState(EVENT_ZOMBIE);
        }
        else
        {
            UpdateState(EVENT_DYING);
        }

		if (!IsSneaked())
		{
			combat_->UnRegisterATB(xid_.id);
		}

		combat_->TestCombatEnd();
	}

	OnDamage(damage, attacker);
}

void CombatPlayer::HandleMSG(const MSG& msg)
{
	switch (msg.message)
	{
		default:
		{
			assert(false && "未知消息类型");
		}
		break;
	};
}

void CombatPlayer::SendPacket(const shared::net::ProtoPacket& packet) const
{
	Combat::CombatSenderCB(role_.roleid, combat_->GetCombatID(), packet);
}

void CombatPlayer::ResetState()
{
    unit_state_.Clear();
    unit_state_.Init(UT_PLAYER);
}

bool CombatPlayer::HandleCMD(const PlayerCMD& cmd)
{
	switch (cmd.cmd_no)
	{
		case COMBAT_CMD_SELECT_SKILL:
		{
			return CMDHandler_SelectSkill(cmd);
		}
		break;
		case COMBAT_CMD_PET_ATTACK:
		{
			return CMDHandler_PetAttack(cmd);
		}
		case COMBAT_CMD_TERMINATE_COMBAT:
		{
			return CMDHandler_TerminateCombat(cmd);
		}
        case COMBAT_CMD_PLAYER_ONLINE:
        {
            return CMDHandler_PlayerOnline(cmd);
        }
        case COMBAT_CMD_PLAYER_OFFLINE:
        {
            return CMDHandler_PlayerOffline(cmd);
        }
		default:
		break;
	}
	return false;
}

bool CombatPlayer::CMDHandler_SelectSkill(const PlayerCMD& cmd)
{
    if (cmd.params.size() != 2)
    {
        return false;
    }
    int32_t skillid = cmd.params[0];
	if (skillid <= 0 || !IsSkillExist(skillid))
	{
		__PRINTF("玩家 %ld 选择技能 %d 失败", cmd.roleid, skillid);
		return false;
	}

    //通知玩家本人
	cur_skill_ = skillid;
    if (cmd.params[1] != 0)
    {
        G2C::CombatSelectSkill_Re packet;
        packet.result = G2C::CombatSelectSkill_Re::ERR_SUCCESS;
        packet.cur_skill = cur_skill_;
        SendPacket(packet);

        if (combat_->IsStateWaitSelectSkill())
        {
            combat_->WaitSelectSkill(0, 0, 0);
        }
    }

	__PRINTF("玩家 %d 选择技能 %d", GetID(), cur_skill_);
    return true;
}

bool CombatPlayer::CMDHandler_PetAttack(const PlayerCMD& cmd)
{
    if (cmd.params.size() != 1)
    {
        return false;
    }
    int32_t pet_combat_pos = cmd.params[0];
	if (pet_combat_pos < 0)
		return false;

	if (!CanCastPetSkill())
		return false;

	if (!pet_man_.Attack(pet_combat_pos))
		return false;

	G2C::CombatPetAttack_Re packet;
	packet.result = 1;
	packet.combat_pet_inv_pos = pet_combat_pos;
	SendPacket(packet);

    if (combat_->IsStateWaitSelectPet())
    {
        combat_->WaitSelectPet(-1, 0, 0);
    }
	return true;
}

bool CombatPlayer::CMDHandler_TerminateCombat(const PlayerCMD& cmd)
{
	//终止战斗是无条件的,所以这里不判定了
	return combat_->Terminate();
}

const CombatAward& CombatPlayer::GetAward() const
{
	return combat_award_;
}

void CombatPlayer::GainCombatExp(int exp)
{
	if (exp <= 0) return;
	combat_award_.exp += exp;
}

void CombatPlayer::GainCombatMoney(int money)
{
	if (money <= 0) return;
	combat_award_.money += money;
}

void CombatPlayer::GainCombatItem(int item_id, int item_count)
{
	if (item_id <= 0 && item_count <= 0)
		return;

	ItemEntry __item;
	__item.item_id = item_id;
	__item.item_count = item_count;
	combat_award_.items.push_back(__item);
}

void CombatPlayer::GainLotteryItem(int item_id, int item_count)
{
	if (item_id <= 0 && item_count <= 0)
		return;

	ItemEntry __item;
	__item.item_id = item_id;
	__item.item_count = item_count;
	combat_award_.lottery.push_back(__item);
}

void CombatPlayer::GainCombatAward(const std::vector<ItemEntry>& items, int32_t money, int32_t exp)
{
	GainCombatMoney(money);
	GainCombatExp(exp);
	for (size_t i = 0; i < items.size(); ++ i)
	{
		GainCombatItem(items[i].item_id, items[i].item_count);
	}
}

void CombatPlayer::KillMob(UnitID unit_id, TemplID mob_tid)
{
    for (size_t i = 0; i < mob_list_killed_.size(); ++ i)
    {
        mob_killed& entry = mob_list_killed_[i];
        if (entry.unit_id == unit_id)
        {
            //已经统计过了
            return;
        }
    }

    mob_killed entry;
    entry.unit_id = unit_id;
    entry.mob_tid = mob_tid;
    mob_list_killed_.push_back(entry);
}

void CombatPlayer::KillPlayer(UnitID unit_id, RoleID roleid)
{
    player_killed entry;
    entry.unit_id = unit_id;
    entry.roleid = roleid;
    player_list_killed_.push_back(entry);
}

void CombatPlayer::GetMobsKilled(std::vector<MobKilled>& mob_killed_vec) const
{
    std::map<TemplID, int> map;
    for (size_t i = 0; i < mob_list_killed_.size(); ++ i)
    {
        const mob_killed& entry = mob_list_killed_[i];
        map[entry.mob_tid] ++;
    }

    std::map<TemplID, int>::const_iterator it = map.begin();
    for (; it != map.end(); ++ it)
    {
        MobKilled entry;
        entry.mob_tid = it->first;
        entry.mob_count = it->second;
        mob_killed_vec.push_back(entry);
    }
}

std::vector<RoleID> CombatPlayer::GetPlayersKilled() const
{
    std::vector<RoleID> player_list;
    for (size_t i = 0; i < player_list_killed_.size(); ++ i)
    {
        player_list.push_back(player_list_killed_[i].roleid);
    }
    return player_list;
}

bool CombatPlayer::TestCoolDown(int cooldown_id) const
{
	return cooldown_man_.TestCoolDown(cooldown_id);
}

bool CombatPlayer::SetCoolDown(int cooldown_id, int interval)
{
	return cooldown_man_.SetCoolDown(cooldown_id, interval);
}

void CombatPlayer::SetRoleInfo(int64_t roleid, const std::string& rolename, char cls, char gender, short level, TemplID weapon_id)
{
	role_.roleid   = roleid;
	role_.rolename = rolename;
	role_.cls      = cls;
	role_.level    = level;
	role_.gender   = gender;
	role_.weapon_id = weapon_id;

	SetLevel(level);
}

void CombatPlayer::GenPetPower()
{
	pet_man_.GenPower();
}

UnitID CombatPlayer::GetAttackPetID() const
{
	return pet_man_.GetAttackPetID();
}

bool CombatPlayer::IsSkillExist(SkillID skill_id) const
{
	if (skill_id == default_skill_)
		return true;

	SkillTreeVec::const_iterator it = std::find_if(skill_tree_vec_.begin(), skill_tree_vec_.end(), skill_finder(skill_id));
	return it != skill_tree_vec_.end();
}

bool CombatPlayer::IncSkillLevel(TemplID sk_tree_id, int32_t lvl)
{
	SkillTreeVec::iterator it = std::find_if(skill_tree_vec_.begin(), skill_tree_vec_.end(), sk_tree_finder(sk_tree_id));
	if (it == skill_tree_vec_.end())
	{
		assert(false);
		return false;
	}

	//更新临时等级
	int32_t __level = it->level + it->tmp_level + lvl;
	if (__level > it->max_level)
	{
		it->tmp_level = it->max_level - it->level;
	}
	else
	{
		it->tmp_level += lvl;
	}

	//更新调用技能
	int32_t level  = it->level + it->tmp_level;
	it->skill_id = GetSkillByLevel(sk_tree_id, level);
	AddSkill(it->skill_id);
	return true;
}

bool CombatPlayer::DecSkillLevel(TemplID sk_tree_id, int32_t lvl)
{
	SkillTreeVec::iterator it = std::find_if(skill_tree_vec_.begin(), skill_tree_vec_.end(), sk_tree_finder(sk_tree_id));
	if (it == skill_tree_vec_.end())
	{
		assert(false);
		return false;
	}

	if (it->tmp_level < lvl)
	{
		assert(false);
		return false;
	}

	//更新临时等级
	it->tmp_level -= lvl;

	//更新调用技能
	int32_t level  = it->level + it->tmp_level;
	it->skill_id = GetSkillByLevel(sk_tree_id, level);
	AddSkill(it->skill_id);
	return true;
}

void CombatPlayer::SetGenPowerFlag()
{
	gen_mp_flag_ = true;
}

void CombatPlayer::Revive()
{
	SetKiller(0);
	UpdateState(EVENT_REVIVE);
	combat_->RegisterATB(this);

	//更新魔偶状态
	if (cur_golem_ && cur_golem_->CanSummoned())
	{
		cur_golem_->OnActived();
	}

	G2C::CombatPlayerState packet;
	packet.unit_id = GetID();
	packet.state   = G2C::COMBAT_PLAYER_STATE_NORMAL;
	BroadCastCMD(packet);

    __PRINTF("----------------------玩家复活了, unit_id=%d", xid_.id);
}

void CombatPlayer::Resurrect(int32_t scale)
{
	if (!IsDead())
		return;

	SetKiller(0);
	SetHP(GetMaxHP() * (scale / 100));
	UpdateState(EVENT_REVIVE);
	combat_->RegisterATB(this);

	//更新魔偶状态
	if (cur_golem_ && cur_golem_->CanSummoned())
	{
		cur_golem_->OnActived();
	}

	G2C::CombatPlayerState packet;
	packet.unit_id = GetID();
	packet.state   = G2C::COMBAT_PLAYER_STATE_NORMAL;
	BroadCastCMD(packet);
}

char CombatPlayer::GetStatus() const
{
	int rst = -1;
	if (IsDead()) rst = G2C::COMBAT_PLAYER_STATE_DEAD;
	else if (IsDying()) rst = G2C::COMBAT_PLAYER_STATE_DYING;
	else rst = G2C::COMBAT_PLAYER_STATE_NORMAL;
	return rst;
}

int CombatPlayer::CalDamageValue(const skill::SkillDamage& damage) const
{
#define CAL_DAMAGE(player_damage_list) \
	{ \
		for (size_t k = 0; k < player_damage_list.size(); ++ k) \
		{ \
			const skill::PlayerDamage& player_damage = player_damage_list[k]; \
			if (player_damage.defender != GetID()) \
			{ \
				for (size_t m = 0; m < player_damage.dmgs.size(); ++ m) \
				{ \
					const skill::EffectDamage& effect_damage = player_damage.dmgs[m]; \
					for (size_t n = 0; n < effect_damage.dmgs.size(); ++ n) \
					{ \
						const skill::Damage& __damage = effect_damage.dmgs[n]; \
						if (__damage.type == 0 && __damage.prop == PROP_INDEX_HP && __damage.value < 0) \
						{ \
							damage_value -= __damage.value; \
						} \
					} \
				} \
			} \
		} \
	}

	int32_t damage_value = 0;
	for (size_t j = 0; j < damage.frames.size(); ++ j)
	{
		const skill::FrameDamage& frame_damage = damage.frames[j];
		CAL_DAMAGE(frame_damage.players);
		CAL_DAMAGE(frame_damage.redir_players);
#undef CAL_DAMAGE
	}

	return damage_value;
}

void CombatPlayer::Trace() const
{
	CombatUnit::Trace();

	__PRINTF("角色ID：%ld, 角色名：%s, 职业：%d, 等级：%d, 武器ID：%d",
				role_.roleid,
				role_.rolename.c_str(),
				role_.cls,
				role_.level,
				role_.weapon_id);

	__PRINTF("附加技能信息：");
    SkillSet::const_iterator ait = addon_skills_.begin();
    for (; ait != addon_skills_.end(); ++ait)
    {
		__PRINTF("\t\t附加技能：%d", *ait);
    }

	__PRINTF("技能树信息：");
	for (size_t i = 0; i < skill_tree_vec_.size(); ++ i)
	{
		const SkillTree& st = skill_tree_vec_[i];
		__PRINTF("\t\tsk_tree_id(%d), skill_id(%d), cur_lvl(%d), tmp_lvl(%d), max_lvl(%d)",
				st.skill_tree_id,
				st.skill_id,
				st.level,
				st.tmp_level,
				st.max_level);
	}

	pet_man_.Trace();
}

}; // namespace combat
