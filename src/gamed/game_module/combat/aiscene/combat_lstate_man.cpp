#include "combat_lstate_man.h"
#include "combat_def.h"
#include "combat_lua_API.h"
#include "extra_templ/battle_scene.h"
#include "extra_templ/extratempl_man.h"

namespace combat
{

struct CFunction
{
	const char* func_name;
	lua_CFunction func;
};

static CFunction cfunction_table[] =
{
	{"Sleep", combat::CombatLuaAPI::Sleep},
	{"MobSpeak", combat::CombatLuaAPI::MobSpeak},
	{"MultiMobSpeak", combat::CombatLuaAPI::MultiMobSpeak},
	{"PlayerSpeak", combat::CombatLuaAPI::PlayerSpeak},
	{"PlayerLearnSkill", combat::CombatLuaAPI::PlayerLearnSkill},
	{"SummonMob", combat::CombatLuaAPI::SummonMob},
	{"SummonNpc", combat::CombatLuaAPI::SummonNpc},
	{"SelectSkill", combat::CombatLuaAPI::SelectSkill},
	{"CastInstantSkill", combat::CombatLuaAPI::CastInstantSkill},
	{"TerminateCombat", combat::CombatLuaAPI::TerminateCombat},
	{"GetTotalRoundCount", combat::CombatLuaAPI::GetTotalRoundCount},
	{"WaitSelectSkill", combat::CombatLuaAPI::WaitSelectSkill},
	{"WaitSelectPet", combat::CombatLuaAPI::WaitSelectPet},
	{"Shake", combat::CombatLuaAPI::Shake},
	{"GetUnitProp", combat::CombatLuaAPI::GetUnitProp},
	{"GetUnitType", combat::CombatLuaAPI::GetUnitType},
};


/****************************CombatLStateMan::CLuaState*******************************/
/****************************CombatLStateMan::CLuaState*******************************/
/****************************CombatLStateMan::CLuaState*******************************/
/****************************CombatLStateMan::CLuaState*******************************/
bool CLuaState::Init(const char* file)
{
	//init lua_State
	lstate = luaL_newstate();
	luaopen_base(lstate);

	//load lua-script
	if (!Load(file))
	{
		return false;
	}

	//register C-function
	for (size_t i = 0; i < sizeof(combat::cfunction_table) / sizeof(CFunction); ++ i)
	{
		CFunction& func = cfunction_table[i];
		lua_register(lstate, func.func_name, func.func);
	}
	return true;
}

bool CLuaState::Load(const char* file)
{
	//load lua-script
	if (luaL_loadfile(lstate, file) != 0)
	{
		__PRINTF("luaL_loadfile %s error!!!", file);
		return false;
	}

	//start lua-coroutine
	int ret = lua_resume(lstate, NULL, 0);
	if (ret != LUA_OK && ret != LUA_YIELD)
	{
		__PRINTF("lua_resume error!!! ret=%d, %s", ret, lua_tostring(lstate, -1));
		lua_pop(lstate, -1);
		return false;
	}
	return true;
}

/***************************CombatLStateMan******************************/
/***************************CombatLStateMan******************************/
/***************************CombatLStateMan******************************/
/***************************CombatLStateMan******************************/
void CombatLStateMan::Initialize(const std::string& path)
{
	shared::MutexLockGuard keeper(lock_);

	scene_script_path_ = path;

	//query all battle-scene template
	std::vector<const extraTempl::BattleSceneTempl*> list;
	s_pExtraTempl->QueryExtraTemplByType(list);

	//cache scene-id to its index
	for (size_t i = 0, index = 0; i < list.size(); ++ i)
	{
		const extraTempl::BattleSceneTempl* tpl = list[i];
		if (tpl->scene_event_id > 0)
		{
			id2idx_map_[tpl->templ_id] = index ++;
		}
	}

	//resize lstate_pool
	lstate_pool_.resize(id2idx_map_.size());

	//pre-alloc lua_State
	Id2IndexMap::const_iterator it = id2idx_map_.begin();
	for (; it != id2idx_map_.end(); ++ it)
	{
		TemplID scene_id = it->first;
		CLuaState* lstate = AllocLuaState(scene_id);
		if (lstate)
		{
			InsertLuaState(lstate);
		}
	}
}

void CombatLStateMan::Release()
{
	shared::MutexLockGuard keeper(lock_);

	LuaStateMap::iterator it = lstate_map_.begin();
	for (; it != lstate_map_.end(); ++ it)
	{
		CLuaState* lstate = it->second;
		lstate->Release();
		delete lstate;
	}

	for (size_t idx = 0; idx < lstate_pool_.size(); ++ idx)
	{
		lstate_pool_[idx].clear();
	}

	id2idx_map_.clear();
	lstate_pool_.clear();
	lstate_map_.clear();
}

static std::string FilePath(const std::string& path, int32_t combat_scene_id)
{
	char scene_id[128];
	memset(scene_id, 0, sizeof(scene_id)/sizeof(char));
	sprintf(scene_id, "%d", combat_scene_id);

	//build ai-script file-name
	std::string __file_path = path;
	__file_path.append("scene/combat_scene_ai_");
	__file_path.append(scene_id);
	__file_path.append(".lua");

	return __file_path;
}

CLuaState* CombatLStateMan::Alloc(int32_t combat_scene_id)
{
	shared::MutexLockGuard keeper(lock_);

	Id2IndexMap::const_iterator it = id2idx_map_.find(combat_scene_id);
	if (it == id2idx_map_.end())
	{
		__PRINTF("invalid combat-scene-id：%d", combat_scene_id);
		return NULL;
	}

	int index = it->second;
	CLuaState* __lstate = NULL;
	if (!lstate_pool_[index].empty())
	{
		//OK, use directly
		__lstate = lstate_pool_[index].front();
		RemoveFromMap(__lstate);
		RemoveFromPool(__lstate);
	}
	else if (!lstate_map_.empty())
	{
		//exist lua_State, check whether can use.
		LuaStateMap::iterator it_lstate = lstate_map_.begin();
		for (; it_lstate != lstate_map_.end(); ++ it_lstate)
		{
			CLuaState* state = it_lstate->second;
			if (state->count < MAX_SCENE_SCRIPT_COUNT)
			{
				//found
				__lstate = state;
				break;
			}
		}

		if (__lstate)
		{
			//OK, load combat-scene-script.
			std::string file_path = FilePath(scene_script_path_, combat_scene_id);
			if (!__lstate->Load(file_path.c_str()))
			{
				__PRINTF("load scene script failed, scene_id: %d", combat_scene_id);

				__lstate->Release();
				RemoveFromMap(__lstate);
				RemoveFromPool(__lstate);
				return NULL;
			}

			__lstate->SetMask(index);
			RemoveFromMap(__lstate);
			RemoveFromPool(__lstate);
		}
		else
		{
			//failed, because can't load more scene-script
			//alloc new lua_State
			CLuaState* state = AllocLuaState(combat_scene_id);
			if (!state)
			{
				return NULL;
			}

			__lstate = state;
		}
	}
	else
	{
		//alloc new lua_State
		__lstate = AllocLuaState(combat_scene_id);
		if (!__lstate)
		{
			return NULL;
		}
	}

	assert(__lstate->IsStateOK());
	return __lstate;
}

void CombatLStateMan::Free(CLuaState* lstate)
{
	shared::MutexLockGuard keeper(lock_);

	InsertLuaState(lstate);
}

CLuaState* CombatLStateMan::AllocLuaState(TemplID combat_scene_id)
{
	Id2IndexMap::const_iterator it = id2idx_map_.find(combat_scene_id);
	if (it == id2idx_map_.end())
	{
		__PRINTF("invalid combat-scene-id：%d", combat_scene_id);
		return NULL;
	}

	std::string file_path = FilePath(scene_script_path_, combat_scene_id);
    if (::access(file_path.c_str(), R_OK))
    {
        __PRINTF("%s isn't exist!", file_path.c_str());
        return NULL;
    }

	CLuaState* lstate = new CLuaState();
	lstate->SetID(++ id_counter_);
	lstate->SetMask(it->second);

	if (!lstate->Init(file_path.c_str()))
	{
		__PRINTF("load scene script failed, scene_id: %d", combat_scene_id);
		lstate->Release();
		delete lstate;
		return NULL;
	}
	return lstate;
}

void CombatLStateMan::InsertLuaState(CLuaState* lstate)
{
	//检查lua_State状态
	assert(lstate->IsStateOK());
	assert(lstate->count <= MAX_SCENE_SCRIPT_COUNT);

	lstate_map_[lstate->id] = lstate;

    int n     = 0;
    int idx   = 0;
    int count = lstate->mask.count();
    int size  = lstate->mask.size();
    while (n < count && idx < size)
    {
        if (lstate->mask.test(idx))
        {
            //重新插入链表
            LuaStateList& list = lstate_pool_[idx];
            list.push_back(lstate);
            list.sort(LStateCMP());

            ++ n;
        }

        ++ idx;
    }
}

void CombatLStateMan::RemoveFromMap(CLuaState* lstate)
{
	int32_t id = lstate->id;
	LuaStateMap::iterator it = lstate_map_.find(id);
	assert(it != lstate_map_.end());
	lstate_map_.erase(it);
}

void CombatLStateMan::RemoveFromPool(CLuaState* lstate)
{
    int n     = 0;
    int idx   = 0;
    int count = lstate->mask.count();
    int size  = lstate->mask.size();
    while (n < count && idx < size)
    {
        if (lstate->mask.test(idx))
        {
            //删除链表上的lus_State
            LuaStateList& list = lstate_pool_[idx];
            for (LuaStateList::iterator it = list.begin(); it != list.end(); ++ it)
            {
                if ((*it) == lstate)
                {
                    list.erase(it);
                    break;
                }
            }

            ++ n;
        }

        ++ idx;
    }
}

void CombatLStateMan::Trace() const
{
	if (lstate_map_.empty())
		return;

	__PRINTF("----------------------combat-lua-state-man------------------------");
	LuaStateMap::const_iterator it = lstate_map_.begin();
	for (; it != lstate_map_.end(); ++ it)
	{
		const CLuaState* __lstate = it->second;
		__PRINTF("id = %d, count = %d", __lstate->id, __lstate->count);
	}
}

};
