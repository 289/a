#include "combat.h"
#include "combat_scene_ai.h"
#include "combat_lua_API.h"
#include "combat_lstate_man.h"
#include "extra_templ/battle_scene.h"
#include "extra_templ/extratempl_man.h"

namespace combat
{

const std::string battle_scene_script_dir = "scene/combat_scene_ai_";

CombatSceneAI::CombatSceneAI(Combat* combat):
	combat_(combat),
	lstate_(NULL)
{
}

CombatSceneAI::~CombatSceneAI()
{
	if (lstate_)
	{
		Release();
	}
}

bool CombatSceneAI::Init()
{
	int32_t combat_scene_id = combat_->GetCombatSceneID();
	lstate_ = s_combat_lstate_man.Alloc(combat_scene_id);
	if (!lstate_)
		return false;
	return true;
}

void CombatSceneAI::Release()
{
	lua_State* __lstate = lstate_->lua_state();
	int status = lua_status(__lstate);
	assert(status == LUA_OK || status == LUA_YIELD);
	if (status != LUA_OK)
	{
		for (;;)
		{
			int ret = lua_resume(__lstate, NULL, 0);
			if (ret == LUA_OK) break;
			else if (ret == LUA_YIELD) continue;
			else { assert(false); break; }
		}
	}

	s_combat_lstate_man.Free(lstate_);
	lstate_ = NULL;
}

bool CombatSceneAI::IsNormalStatus() const
{
	if (!lstate_) return false;
	lua_State* __lstate = lstate_->lua_state();
	return lua_status(__lstate) == LUA_OK;
}

bool CombatSceneAI::IsYieldStatus() const
{
	if (!lstate_) return false;
	lua_State* __lstate = lstate_->lua_state();
	return lua_status(__lstate) == LUA_YIELD;
}

#define GET_COMBAT_ID_STR char combat_id_str[32] = {0}; sprintf(combat_id_str, "%d", combat_->GetCombatSceneID());
#define CHECK_LSTATE_IS_NOT_NULL(lstate) if (!lstate) return;
#define ASSERT_LSTATE_IS_NOT_NULL(lstate) assert(lstate);
#define ASSERT_LSTATE_STATUS_OK(lstate) assert(lua_status(lstate->lua_state()) == LUA_OK)

void CombatSceneAI::OnCombatStart()
{
	CHECK_LSTATE_IS_NOT_NULL(lstate_);
	ASSERT_LSTATE_STATUS_OK(lstate_);

	lua_State* __lstate = lstate_->lua_state();

	//build lua-func name
	GET_COMBAT_ID_STR;
	std::string lua_fun_name("onCombatStart_");
	lua_fun_name.append(combat_id_str);

	//push func and param to lua-stack
	lua_getglobal(__lstate, lua_fun_name.c_str());
	lua_pushnumber(__lstate, combat_->GetID());

	//query monster-tid-list
	std::vector<int32_t> mob_tid_list;
	combat_->GetMobList(mob_tid_list);

	//push mob-tid-list to lua-stack
	lua_newtable(__lstate);
	for (size_t i = 0; i < mob_tid_list.size(); ++ i)
	{
		lua_pushnumber(__lstate, mob_tid_list[i]);
		lua_rawseti(__lstate, -2, i+1);
	}

	//query role-cls-list
	std::vector<int> role_cls_list;
	combat_->GetRoleClsList(role_cls_list);

	//push role-cls-list to lua-stack
	lua_newtable(__lstate);
	for (size_t i = 0; i < role_cls_list.size(); ++ i)
	{
		lua_pushnumber(__lstate, role_cls_list[i]);
		lua_rawseti(__lstate, -2, i+1);
	}

	//call lua-function
	int ret = lua_resume(__lstate, NULL, 3);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("%s, lua_resume failed!!! ret=%d, %s", lua_fun_name.c_str(), ret, lua_tostring(__lstate, -1));
		lua_pop(__lstate, -1);
	}
}

void CombatSceneAI::OnCombatEnd()
{
	CHECK_LSTATE_IS_NOT_NULL(lstate_);
	ASSERT_LSTATE_STATUS_OK(lstate_);

	lua_State* __lstate = lstate_->lua_state();

	//build lua-func name
	GET_COMBAT_ID_STR;
	std::string lua_fun_name("onCombatEnd_");
	lua_fun_name.append(combat_id_str);

	//push lua-func and param onto lua-stack
	lua_getglobal(__lstate, lua_fun_name.c_str());
	lua_pushnumber(__lstate, combat_->GetID());

	//call lua-function
	int ret = lua_resume(__lstate, NULL, 1);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("%s, lua_resume failed!!! ret=%d, %s", lua_fun_name.c_str(), ret, lua_tostring(__lstate, -1));
		lua_pop(__lstate, -1);
	}
}

void CombatSceneAI::OnDamage(int32_t mob_uid, int32_t mob_tid)
{
	CHECK_LSTATE_IS_NOT_NULL(lstate_);
	ASSERT_LSTATE_STATUS_OK(lstate_);

	lua_State* __lstate = lstate_->lua_state();

	//build lua-func name
	GET_COMBAT_ID_STR;
	std::string lua_fun_name("onDamage_");
	lua_fun_name.append(combat_id_str);

	//push lua-func and param onto lua-stack
	lua_getglobal(__lstate, lua_fun_name.c_str());
	lua_pushnumber(__lstate, combat_->GetID());
	lua_pushnumber(__lstate, mob_uid);
	lua_pushnumber(__lstate, mob_tid);

	//continue excute lua-script
	int ret = lua_resume(__lstate, NULL, 3);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("%s, lua_resume failed!!! ret=%d, %s", lua_fun_name.c_str(), ret, lua_tostring(__lstate, -1));
		lua_pop(__lstate, -1);
	}
}

void CombatSceneAI::OnRoundStart(int32_t unit_id)
{
	CHECK_LSTATE_IS_NOT_NULL(lstate_);
	ASSERT_LSTATE_STATUS_OK(lstate_);

	lua_State* __lstate = lstate_->lua_state();

	//build lua-func name
	GET_COMBAT_ID_STR;
	std::string lua_fun_name("onRoundStart_");
	lua_fun_name.append(combat_id_str);

	//push lua-func and param onto lua-stack
	lua_getglobal(__lstate, lua_fun_name.c_str());
	lua_pushnumber(__lstate, combat_->GetID());
	lua_pushnumber(__lstate, unit_id);
	lua_pushnumber(__lstate, combat_->GetRoundCount());

	//query role-cls-list
	std::vector<int> role_cls_list;
	combat_->GetRoleClsList(role_cls_list);

	//push role-cls-list to lua-stack
	lua_newtable(__lstate);
	for (size_t i = 0; i < role_cls_list.size(); ++ i)
	{
		lua_pushnumber(__lstate, role_cls_list[i]);
		lua_rawseti(__lstate, -2, i+1);
	}

	//continue excute lua-script
	int ret = lua_resume(__lstate, NULL, 4);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("%s, lua_resume failed!!! ret=%d, %s", lua_fun_name.c_str(), ret, lua_tostring(__lstate, -1));
		lua_pop(__lstate, -1);
	}
}

void CombatSceneAI::OnRoundEnd(int32_t unit_id)
{
	CHECK_LSTATE_IS_NOT_NULL(lstate_);
	ASSERT_LSTATE_STATUS_OK(lstate_);

	lua_State* __lstate = lstate_->lua_state();

	//build lua-func name
	GET_COMBAT_ID_STR;
	std::string lua_fun_name("onRoundEnd_");
	lua_fun_name.append(combat_id_str);

	//push lua-func and param onto lua-stack
	lua_getglobal(__lstate, lua_fun_name.c_str());
	lua_pushnumber(__lstate, combat_->GetID());
	lua_pushnumber(__lstate, unit_id);
	lua_pushnumber(__lstate, combat_->GetRoundCount());

	//continue excute lua-script
	int ret = lua_resume(__lstate, NULL, 3);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("%s, lua_resume failed!!! ret=%d, %s", lua_fun_name.c_str(), ret, lua_tostring(__lstate, -1));
		lua_pop(__lstate, -1);
	}
}

bool CombatSceneAI::Resume()
{
	ASSERT_LSTATE_IS_NOT_NULL(lstate_);

    if (lua_status(lstate_->lua_state()) != LUA_YIELD)
        return true;

	lua_State* __lstate = lstate_->lua_state();
	int ret = lua_resume(__lstate, NULL, 0);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("Wakeup, lua_resume failed!!! ret=%d, %s", ret, lua_tostring(__lstate, -1));
		return false;
	}

	return ret == LUA_OK;
}

#undef GET_COMBAT_ID_STR
#undef CHECK_LSTATE_IS_NOT_NULL
#undef ASSERT_LSTATE_IS_NOT_NULL
#undef ASSERT_LSTATE_STATUS_OK
}
