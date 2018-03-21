#ifndef __GAME_MODULE_COMBAT_SCENE_AI_LSTATE_MAN_H__
#define __GAME_MODULE_COMBAT_SCENE_AI_LSTATE_MAN_H__

#include <map>
#include <list>
#include <vector>
#include <bitset>
#include <string>
#include <lua.hpp>

#include "combat_types.h"

#include "shared/base/mutex.h"
#include "shared/base/singleton.h"

namespace combat
{

/**
 * @class CLuaState
 * @brief 对lua_State的封装
 */
struct CLuaState
{
	lua_State* lstate;      //lua栈
    std::bitset<1024> mask; //加载了哪些脚本
	uint32_t   id;          //分配的序列号
	uint16_t   count;       //加载了多少个脚本

	CLuaState():
		lstate(NULL),
		id(0),
		count(0)
	{}
	~CLuaState()
	{}

	bool Init(const char* file);
	bool Load(const char* file);
	void SetID(int id);
	void SetMask(int index);
	bool IsStateOK() const;
	void Release();

	lua_State* lua_state();
};

/**
 * @class CombatLStateMan
 * @brief 战斗场景策略脚本的lua_State管理器
 */
class CombatLStateMan : public shared::Singleton<CombatLStateMan>
{
public:
private:
	typedef std::map<TemplID/*scene-id*/, int>         Id2IndexMap;  // combat-scene-id to index
	typedef std::map<int/*lua-state-id*/, CLuaState*>  LuaStateMap;  // availiable CLuaState map
	typedef std::list<CLuaState*>                      LuaStateList; // CLuaState link-list
	typedef std::vector<LuaStateList>                  LuaStatePool; // availiable CLuaState pool

	Id2IndexMap  id2idx_map_;
	LuaStateMap  lstate_map_;
	LuaStatePool lstate_pool_;

	int32_t      id_counter_;
	std::string  scene_script_path_;

	shared::MutexLock lock_;

	struct LStateCMP
	{
		bool operator() (const CLuaState* lhs, const CLuaState* rhs) const
		{
			return lhs->count <= rhs->count;
		}
	};

public:
	CombatLStateMan():
		id_counter_(0)
	{}
	virtual ~CombatLStateMan()
	{}

	static CombatLStateMan& GetInstance()
	{
		static CombatLStateMan instance;
		return instance;
	}

	// ---- thread safe ----
	void Initialize(const std::string& path);
	void Release();
	void Trace() const;

	// ---- thread safe ----
	CLuaState* Alloc(TemplID scene_id); 
	void Free(CLuaState* lstate);


private:

	// ---- thread safe ----
	CLuaState* AllocLuaState(TemplID scene_id);

	// ---- thread unsafe ----
	void InsertLuaState(CLuaState* lstate);
	void RemoveFromMap(CLuaState* lstate);
	void RemoveFromPool(CLuaState* lstate);
};


///
/// inline func
///
inline void CLuaState::SetID(int _id)
{
	id = _id;
}

inline void CLuaState::SetMask(int idx)
{
	assert(idx <= 64);
    mask.set(idx);
	++ count;
}

inline bool CLuaState::IsStateOK() const
{
	return lua_status(lstate) == LUA_OK;
}

inline lua_State* CLuaState::lua_state()
{
	return lstate;
}

inline void CLuaState::Release()
{
	lua_close(lstate);
}

#define s_combat_lstate_man CombatLStateMan::GetInstance()

};

#endif // __GAME_MODULE_COMBAT_SCENE_AI_LSTATE_MAN_H__
