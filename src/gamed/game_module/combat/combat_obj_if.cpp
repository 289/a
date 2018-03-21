#include "combat_types.h"
#include "combat_obj_if.h"
#include "combat_unit.h"
#include "combat_player.h"
#include "combat_npc.h"

namespace combat
{

CombatObjInterface::CombatObjInterface(CombatUnit* unit)
	: obj(unit)
{
}

CombatObjInterface::~CombatObjInterface()
{
	obj = NULL;
}

#define CAST_TO_PLAYER(obj) \
		assert(obj->IsPlayer()); \
		CombatPlayer* player = dynamic_cast<CombatPlayer*>(obj);

CombatUnit* CombatObjInterface::GetParent() const
{
	return obj;
}

bool CombatObjInterface::IsDead() const
{
    return obj->IsDead() || obj->IsZombie();
}

bool CombatObjInterface::IsDying() const
{
	return obj->IsDying();
}

bool CombatObjInterface::IsDizzy() const
{
	return obj->IsDizzy();
}

bool CombatObjInterface::IsSleep() const
{
	return obj->IsSleep();
}

bool CombatObjInterface::IsSilent() const
{
	return obj->IsSilent();
}

bool CombatObjInterface::IsStone() const
{
	//return obj->IsSilent();
	return false;
}

bool CombatObjInterface::IsConfuse() const
{
	return obj->IsConfuse();
}

bool CombatObjInterface::IsCharging() const
{
    return obj->IsCharging();
}

bool CombatObjInterface::IsCharged() const
{
    return obj->IsCharged();
}

void CombatObjInterface::SetDizzy(bool dizzy)
{
	obj->SetDizzy(dizzy);
}

void CombatObjInterface::SetSleep(bool sleep)
{
	obj->SetSleep(sleep);
}

void CombatObjInterface::SetSilent(bool silent)
{
	obj->SetSilent(silent);
}

void CombatObjInterface::SetSeal(bool seal)
{
}

void CombatObjInterface::SetStone(bool stone)
{
}

void CombatObjInterface::SetConfuse(bool confuse)
{
	obj->SetConfuse(confuse);
}

void CombatObjInterface::SetCharging(bool charging)
{
    obj->SetCharging(charging);
}

void CombatObjInterface::SetCharged(bool charged)
{
    obj->SetCharged(charged);
}

UnitID CombatObjInterface::GetId() const
{
    /*if (obj->IsNPC())
    {
	    const CombatNpc* npc = dynamic_cast<const CombatNpc*>(obj);
        if (npc != NULL && npc->IsGolem())
        {
            return npc->GetPlayer()->GetID();
        }
    }*/
	return obj->GetID();
}

gamed::ObjInterface* CombatObjInterface::GetOwner()
{
    if (obj->IsNPC())
    {
	    CombatNpc* npc = dynamic_cast<CombatNpc*>(obj);
        if (npc != NULL && npc->IsGolem())
        {
            return npc->GetOwner()->GetObjIf();
        }
    }
	return obj->GetObjIf();
}

int8_t CombatObjInterface::GetLevel() const
{
	return obj->GetLevel();
}

RoleID CombatObjInterface::GetRoleID() const
{
	if (!obj->IsPlayer())
		return -1;
	CAST_TO_PLAYER(obj);
	return player->GetRoleID();
}

int32_t CombatObjInterface::GetRoleClass() const
{
	CAST_TO_PLAYER(obj);
	return player->GetCls();
}

bool CombatObjInterface::IsInCombat() const
{
	return true;
}

int64_t CombatObjInterface::GetCurTime() const
{
	return 0;
}

int8_t CombatObjInterface::GetStatus() const
{
	return 1;
}

int8_t CombatObjInterface::GetBuffStatus() const
{
	return 0;
}

int32_t CombatObjInterface::GetHP()
{
	return obj->GetHP();
}

int32_t CombatObjInterface::GetMP()
{
	return obj->GetMP();
}

int32_t CombatObjInterface::GetEP()
{
	return obj->GetEP();
}

int32_t CombatObjInterface::GetProperty(int32_t index) const
{
	if (index >= 1000)
	{
		//比例提升和降低根据基础属性计算,所以这里做了特殊处理,index>=1000表示请求基础属性
		return obj->GetBaseProp(index-1000);
	}
	else
	{
		return obj->GetProp(index);
	}
}

int32_t CombatObjInterface::GetMaxProp(int32_t index) const
{
	return obj->GetMaxProp(index);
}

void CombatObjInterface::ConsumeProp(int32_t index, int32_t value)
{
	assert(index == PROP_INDEX_MP);
	obj->DecEP(index, value);
}

void CombatObjInterface::DoDamage(int32_t hp, UnitID attacker)
{
	obj->DoDamage(hp, attacker);
}

void CombatObjInterface::DoHeal(int32_t hp, UnitID healer)
{
	obj->DoHeal(hp, healer);
}

void CombatObjInterface::IncPropPoint(int32_t index, int32_t value)
{
	obj->IncProp(index, value);
}

void CombatObjInterface::DecPropPoint(int32_t index, int32_t value)
{
	obj->DecProp(index, value);
}

void CombatObjInterface::IncPropScale(int32_t index, int32_t scale)
{
	obj->IncPropScale(index, scale);
}

void CombatObjInterface::DecPropScale(int32_t index, int32_t scale)
{
	obj->DecPropScale(index, scale);
}

int32_t CombatObjInterface::GetItemCount(TemplID item_id) const
{
	/*
	CAST_TO_PLAYER(obj);
	return player->GetItemCount(item_id);
	*/
	return 0;
}

bool CombatObjInterface::CheckItem(TemplID item_id, int32_t count)
{
	/*
	CAST_TO_PLAYER(obj);
	return player->CheckItem(item_id, count);
	*/
	return false;
}

bool CombatObjInterface::CheckItem(int32_t index, TemplID item_id, int32_t count)
{
	/*
	CAST_TO_PLAYER(obj);
	return player->CheckItem(index, item_id, count);
	*/
	return false;
}

void CombatObjInterface::TakeOutItem(TemplID item_id, int32_t count)
{
	/*
	CAST_TO_PLAYER(obj);
	player->TakeOutItem(item_id, count);
	*/
}

int8_t CombatObjInterface::GetPos() const
{
	return obj->GetPos();
}

bool CombatObjInterface::CanAction() const
{
	return obj->CanAction();
}

bool CombatObjInterface::CanAttacked() const
{
	return obj->CanAttacked();
}

skill::SkillWrapper* CombatObjInterface::GetSkill() const
{
	return obj->GetSkillWrapper();
}

skill::BuffWrapper* CombatObjInterface::GetBuff() const
{
	return obj->GetBuffWrapper();
}

skill::CooldownWrapper* CombatObjInterface::GetCooldown() const
{
	return obj->GetCDWrapper();
}

void CombatObjInterface::GetTeamMate(std::vector<ObjInterface*>& players) const
{
	obj->GetTeamMate(players);
}

void CombatObjInterface::GetEnemy(std::vector<ObjInterface*>& players) const
{
	obj->GetEnemy(players);
}

void CombatObjInterface::GetGolem(std::vector<ObjInterface*>& players) const
{
	obj->GetGolem(players);
}

void CombatObjInterface::IncSkillLevel(TemplID sk_tree_id, int32_t lvl)
{
	CAST_TO_PLAYER(obj);
	player->IncSkillLevel(sk_tree_id, lvl);
}

void CombatObjInterface::DecSkillLevel(TemplID sk_tree_id, int32_t lvl)
{
	CAST_TO_PLAYER(obj);
	player->DecSkillLevel(sk_tree_id, lvl);
}

void CombatObjInterface::AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher)
{
	obj->AttachBuffer(buff_seq, buff_id, attacher);
}

void CombatObjInterface::DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher)
{
	obj->DetachBuffer(buff_seq, buff_id, attacher);
}

bool CombatObjInterface::PlayerSummonGolem(TemplID golem_id)
{
	//CAST_TO_PLAYER(obj);
	//return player->SummonGolem(golem_id);
	return obj->SummonGolem(golem_id);
}

bool CombatObjInterface::PlayerPowerGolem(int32_t power)
{
	//CAST_TO_PLAYER(obj);
	//return player->PowerGolem(power);
	return obj->PowerGolem(power);
}

int32_t CombatObjInterface::GetGolemProp(int32_t golem_id, int8_t index)
{
	//CAST_TO_PLAYER(obj);
	//return player->GetGolemProp(golem_id, index);
	return obj->GetGolemProp(golem_id, index);
}

int32_t CombatObjInterface::GetGolemDeActivePower(int32_t golem_id)
{
	//CAST_TO_PLAYER(obj);
	//return player->GetGolemDeActivePower(golem_id);
	return obj->GetGolemDeActivePower(golem_id);
}

void CombatObjInterface::SetGenPowerFlag()
{
	CAST_TO_PLAYER(obj);
	return player->SetGenPowerFlag();
}

void CombatObjInterface::Revive(int32_t scale)
{
	CAST_TO_PLAYER(obj);
	player->Resurrect(scale);
}

void CombatObjInterface::Trace() const
{
	obj->dump();
}

#undef CAST_TO_PLAYER

}; // namespace combat
