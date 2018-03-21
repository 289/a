#ifndef __GAME_MODULE_COMBAT_AI_NPC_H__
#define __GAME_MODULE_COMBAT_AI_NPC_H__

#include "lua.hpp"
#include "npc_wrapper.h"
#include "npc_lua_state.h"

namespace combat
{

class CombatNpc;
class NpcAI
{
private:
	MobWrapper* wrapper_;
	lua_State*  lstate_;

public:
	NpcAI(CombatNpc* npc):
		lstate_(NULL)
	{
		wrapper_ = new MobWrapper(npc);
		lstate_ = s_npc_lstate_man.QueryLuaState(wrapper_->GetNpcID());
	}

	virtual ~NpcAI()
	{
		if (wrapper_)
		{
			delete wrapper_;
		}

		lstate_ = NULL;
		wrapper_ = NULL;
	}

#define AI_FUNC_DEFINE(mem_func_name, lua_func_name) \
	void mem_func_name() \
	{ \
		if (lstate_) \
		{ \
			std::string __npc_id_str = NpcIdToStr(); \
			std::string __lua_fun_name(#lua_func_name); \
            __lua_fun_name.append("_"); \
			__lua_fun_name.append(__npc_id_str); \
			ELuna::LuaFunction<void> func(lstate_, __lua_fun_name.c_str()); \
			func(wrapper_); \
		} \
	}

    AI_FUNC_DEFINE(OnRoundStart, onRoundStart)
    AI_FUNC_DEFINE(OnRoundEnd, onRoundEnd)
    AI_FUNC_DEFINE(OnDamage, onDamage)
	//AI_FUNC_DEFINE(OnTeammateDead, onTeammateDead)
	void OnTeammateDead()
	{
		if (lstate_)
		{
			std::string __npc_id_str = NpcIdToStr();
			std::string __lua_fun_name("onTeammateDead");
            __lua_fun_name.append("_");
			__lua_fun_name.append(__npc_id_str);
			ELuna::LuaFunction<void> func(lstate_, __lua_fun_name.c_str());
			func(wrapper_);
		}
	}

#undef AI_FUNC_DEFINE

private:
	std::string NpcIdToStr() const
	{
		char npcid[128];
		memset(npcid, 0, sizeof(npcid)/sizeof(char));
		sprintf(npcid, "%d", wrapper_->GetNpcID());
		npcid[strlen(npcid)] = '\0';
		return std::string(npcid);
	}
};

};

#endif // __GAME_MODULE_COMBAT_AI_NPC_H__
