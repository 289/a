#ifndef __GAME_MODULE_COMBAT_LUA_STATE_H__
#define __GAME_MODULE_COMBAT_LUA_STATE_H__

#include <set>
#include <lua.hpp>
#include "eluna.h"
#include "npc_wrapper.h"
#include "shared/base/singleton.h"
#include "data_templ/templ_manager.h"
#include "data_templ/monster_templ.h"


namespace combat
{

const std::string mob_ai_script_dir   = "monster/monster_ai_";
const std::string golem_ai_script_dir = "golem/golem_ai_";
const std::string pet_ai_script_dir   = "pet/pet_ai_";

/**
 * @class NpcLuaState
 * @brief lua_State管理器
 * @brief 服务器启动时初始化，保存怪物策略脚本
 *  注意：这里的lua_State是非线程安全的，并且不可以被挂起
 */
class NpcLuaState : public shared::Singleton<NpcLuaState>
{
private:
	lua_State* lstate_;
	std::set<TemplID/*mob_tid*/> npc_ai_set_;

public:
	NpcLuaState():
		lstate_(NULL)
	{
	}
	virtual ~NpcLuaState()
	{
		if (lstate_)
		{
			ELuna::closeLua(lstate_);
			lstate_ = NULL;
		}

		ELuna::release();
		npc_ai_set_.clear();
	}

	static NpcLuaState& GetInstance()
	{
		static NpcLuaState instance;
		return instance;
	}

	void Initialize(std::string& npc_policy_path)
	{
		//create lua_State
		lstate_ = ELuna::openLua();

		//register global-function
		RegisterCFunction();

		//register c++ class
		ELuna::registerClass<combat::MobWrapper>(lstate_, "MobWrapper", ELuna::constructor<combat::MobWrapper, combat::CombatNpc*>);

		//register c++ method
		RegisterClassMethod();

		std::vector<const dataTempl::MonsterTempl*> list;
		s_pDataTempl->QueryDataTemplByType(list);
		for (size_t i = 0; i < list.size(); ++ i)
		{
			//query all monster template
			const dataTempl::MonsterTempl* tpl = list[i];
			if (tpl->ai_strategy_id <= 0)
				continue;

			//cache monster which has ai-policy
			int32_t mob_templ_id = tpl->templ_id;
			if (!npc_ai_set_.insert(mob_templ_id).second)
			{
				assert(false);
				return;
			}

			//convert npcid to string
			char mob_id[128];
			memset(mob_id, 0, sizeof(mob_id)/sizeof(char));
			sprintf(mob_id, "%d", mob_templ_id);

			//build ai-script file-name
			std::string __lua_file = npc_policy_path;
			__lua_file.append(mob_ai_script_dir);
			__lua_file.append(mob_id);
			__lua_file.append(".lua");

			//load lua-script
			ELuna::doFile(lstate_, __lua_file.c_str());
		}
	}

	lua_State* QueryLuaState(int32_t npc_id)
	{
		if (npc_ai_set_.find(npc_id) != npc_ai_set_.end())
			return lstate_;
		return NULL;
	}

private:
	void RegisterCFunction()
	{
	}

	void RegisterClassMethod()
	{
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetLevel", &combat::MobWrapper::GetLevel);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetHP", &combat::MobWrapper::GetHP);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetMP", &combat::MobWrapper::GetMP);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetEP", &combat::MobWrapper::GetEP);

		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetRoundCounter", &combat::MobWrapper::GetRoundCounter);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetProperty", &combat::MobWrapper::GetProperty);

		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetEnemiesAlive", &combat::MobWrapper::GetEnemiesAlive);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "GetTeammatesAlive", &combat::MobWrapper::GetTeammatesAlive);

		ELuna::registerMethod<combat::MobWrapper>(lstate_, "SetSkill", &combat::MobWrapper::SetSkill);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "Speak", &combat::MobWrapper::Speak);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "CastInstantSkill", &combat::MobWrapper::CastInstantSkill);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "Transform", &combat::MobWrapper::Transform);
		ELuna::registerMethod<combat::MobWrapper>(lstate_, "Escape", &combat::MobWrapper::Escape);
	}
};

#define s_npc_lstate_man NpcLuaState::GetInstance()

};

#endif // __GAME_MODULE_COMBAT_LUA_STATE_H__
