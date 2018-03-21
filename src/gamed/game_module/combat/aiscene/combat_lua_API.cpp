#include "combat_lua_API.h"
#include "combat_man.h"
#include "combat.h"
#include "combat_npc.h"

namespace combat
{

#define CHECK_COMBAT_VALID(combat_id) \
		CombatPVE* combat = dynamic_cast<CombatPVE*>(s_pCombatMan->QueryCombat(combat_id)); \
		if (!combat) \
		{ \
			__PRINTF("无效的战斗ID %d", combat_id); \
			return -1; \
		} \
		if (!combat->IsActived()) \
		{ \
			__PRINTF("战场处于非正常状态, combat_id = %d", combat_id); \
			assert(false); \
			return -1; \
		} \
	    if (!combat->CanScriptOperate()) \
		{ \
			__PRINTF("禁止执行脚本！！！, combat_id = %d", combat_id); \
			return -1; \
		} \
		combat->AssertLocked(); \

int CombatLuaAPI::Sleep(lua_State* lstate)
{
	int32_t msec = luaL_checknumber(lstate, -1);
	int32_t combat_id = luaL_checknumber(lstate, -2);

    CHECK_COMBAT_VALID(combat_id)

	//暂停战斗
	combat->Suspend(msec);

	//挂起脚本执行
	return lua_yield(lstate, 0);
}

int CombatLuaAPI::MobSpeak(lua_State* lstate)
{
	int32_t msec = luaL_checknumber(lstate, -1);
	int32_t id_talk = luaL_checknumber(lstate, -2);
	int32_t mob_uid = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

	CHECK_COMBAT_VALID(combat_id);

	//怪物喊话
	combat->MobSpeak(mob_uid, id_talk, msec);
	return 0;
}

int CombatLuaAPI::MultiMobSpeak(lua_State* lstate)
{
	int32_t msec = luaL_checknumber(lstate, -1);
	int32_t id_talk = luaL_checknumber(lstate, -2);
	int32_t mob_tid = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

	CHECK_COMBAT_VALID(combat_id);

	//全体怪物喊话
	combat->MultiMobSpeak(mob_tid, id_talk, msec);
	return 0;
}

int CombatLuaAPI::PlayerSpeak(lua_State* lstate)
{
	int32_t msec = luaL_checknumber(lstate, -1);
	int32_t id_talk = luaL_checknumber(lstate, -2);
	int32_t combat_id = luaL_checknumber(lstate, -3);

    CHECK_COMBAT_VALID(combat_id);

	combat->PlayerSpeak(id_talk, msec);
	return 0;
}

int CombatLuaAPI::PlayerLearnSkill(lua_State* lstate)
{
	int32_t msec = luaL_checknumber(lstate, -1);
	int32_t lvl_skill_group = luaL_checknumber(lstate, -2);
	int32_t id_skill_group = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

	CHECK_COMBAT_VALID(combat_id);

	//玩家获得一个技能(模拟行为)
	combat->PlayerActivateSkill(id_skill_group, lvl_skill_group, msec);
	return 0;
}

int CombatLuaAPI::SummonMob(lua_State* lstate)
{
	int32_t pos = luaL_checknumber(lstate, -1);
	int32_t mob_tid = luaL_checknumber(lstate, -2);
	int32_t combat_id = luaL_checknumber(lstate, -3);

	CHECK_COMBAT_VALID(combat_id);

	//指定位置召唤怪物
	combat->SummonMob(CombatNpc::TYPE_NPC_MOB, mob_tid, pos);
	return 0;
}

int CombatLuaAPI::SummonNpc(lua_State* lstate)
{
	int32_t pos = luaL_checknumber(lstate, -1);
	int32_t npc_tid = luaL_checknumber(lstate, -2);
	int32_t combat_id = luaL_checknumber(lstate, -3);

	CHECK_COMBAT_VALID(combat_id);

	//指定位置召唤NPC
	combat->SummonMob(CombatNpc::TYPE_NPC_TEAMNPC, npc_tid, pos);
	return 0;
}

int CombatLuaAPI::SelectSkill(lua_State* lstate)
{
	int32_t skill_id = luaL_checknumber(lstate, -1);
	int32_t unit_id = luaL_checknumber(lstate, -2);
	int32_t combat_id = luaL_checknumber(lstate, -3);

	CHECK_COMBAT_VALID(combat_id);

	CombatUnit* unit = combat->Find(unit_id);
	if (!unit)
	{
		__PRINTF("场景脚本选择技能失败, 无效的战斗对象ID(%d)", unit_id);
		return -1;
	}

	if (!unit->IsMob() && !unit->IsTeamNpc())
	{
		__PRINTF("场景脚本选择技能失败, 非怪物战斗对象ID(%d)", unit_id);
		return -1;
	}

	CombatNpc* npc = dynamic_cast<CombatNpc*>(unit);
	npc->SetSkill(skill_id);
	return 0;
}

int CombatLuaAPI::CastInstantSkill(lua_State* lstate)
{
	int32_t skill_id = luaL_checknumber(lstate, -1);
	int32_t mob_tid = luaL_checknumber(lstate, -2);
	int32_t combat_id = luaL_checknumber(lstate, -3);

	CHECK_COMBAT_VALID(combat_id);

	combat->CastInstantSkill(mob_tid, skill_id);
	return 0;
}

int CombatLuaAPI::TerminateCombat(lua_State* lstate)
{
	int32_t combat_id = luaL_checknumber(lstate, -1);

	CHECK_COMBAT_VALID(combat_id);

	combat->Terminate();
	return 0;
}

int CombatLuaAPI::GetTotalRoundCount(lua_State* lstate)
{
	int32_t combat_id = luaL_checknumber(lstate, -1);

	CHECK_COMBAT_VALID(combat_id);

	int32_t round = combat->GetRoundCount();
	lua_settop(lstate, 0);
	lua_pushnumber(lstate, round);
	return 1;
}

int CombatLuaAPI::GetMobList(lua_State* lstate)
{
	//TODO
	return 0;
}

int CombatLuaAPI::WaitSelectSkill(lua_State* lstate)
{
	int32_t skill_index = luaL_checknumber(lstate, -1);
	int32_t msec = luaL_checknumber(lstate, -2);
	int32_t talk_id = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

    CHECK_COMBAT_VALID(combat_id)
	//暂停战斗
	combat->WaitSelectSkill(skill_index, msec, talk_id);

	//挂起脚本执行
	return lua_yield(lstate, 0);
}

int CombatLuaAPI::WaitSelectPet(lua_State* lstate)
{
	int32_t pet_index = luaL_checknumber(lstate, -1);
	int32_t msec = luaL_checknumber(lstate, -2);
	int32_t talk_id = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

    CHECK_COMBAT_VALID(combat_id)
	//暂停战斗
	combat->WaitSelectPet(pet_index, msec, talk_id);

	//挂起脚本执行
	return lua_yield(lstate, 0);
}

int CombatLuaAPI::Shake(lua_State* lstate)
{
	int32_t shake_interval = luaL_checknumber(lstate, -1);
	int32_t shake_duration = luaL_checknumber(lstate, -2);
	int32_t y_amplitude = luaL_checknumber(lstate, -3);
	int32_t x_amplitude = luaL_checknumber(lstate, -4);
	int32_t combat_id = luaL_checknumber(lstate, -5);

    CHECK_COMBAT_VALID(combat_id)

    combat->Shake(x_amplitude, y_amplitude, shake_duration, shake_interval);
    return 0;
}

int CombatLuaAPI::GetUnitProp(lua_State* lstate)
{
	int32_t prop = luaL_checknumber(lstate, -1) - 1;
	int32_t pos = luaL_checknumber(lstate, -2) - 1;
	int32_t party = luaL_checknumber(lstate, -3);
	int32_t combat_id = luaL_checknumber(lstate, -4);

    CHECK_COMBAT_VALID(combat_id)

    int32_t value = 0;
    if (combat->GetUnitProp(party, pos, prop, value) != 0)
    {
        return -1;
    }

	lua_settop(lstate, 0);
	lua_pushnumber(lstate, value);
    return 1;
}

int CombatLuaAPI::GetUnitType(lua_State* lstate)
{
	int32_t unit_id = luaL_checknumber(lstate, -1);
	int32_t combat_id = luaL_checknumber(lstate, -2);

    CHECK_COMBAT_VALID(combat_id)

    int type = combat->GetUnitType(unit_id);
	lua_settop(lstate, 0);
	lua_pushnumber(lstate, type);
    return 1;
}

#undef CHECK_COMBAT_VALID

};
