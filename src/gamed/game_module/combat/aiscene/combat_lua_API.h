#ifndef __GAME_MODULE_COMBAT_LUA_API_H__
#define __GAME_MODULE_COMBAT_LUA_API_H__

#include <lua.hpp>

namespace combat
{

/**
 * @class CombatLuaAPI
 * @brief 供lua脚本调用的全局函数
 */
class CombatLuaAPI
{
public:
	static int Sleep(lua_State* lstate);              // 暂停战斗ATB
	static int MobSpeak(lua_State* lstate);           // 指定怪物怪物喊话
	static int MultiMobSpeak(lua_State* lstate);      // 全体怪物喊话
	static int PlayerSpeak(lua_State* lstate);        // 所有玩家喊话
	static int PlayerLearnSkill(lua_State* lstate);   // 玩家获得技能(模拟学习技能)
	static int SummonMob(lua_State* lstate);          // 召唤怪物
	static int SummonNpc(lua_State* lstate);          // 召唤NPC
	static int SelectSkill(lua_State* lstate);        // 选择技能
	static int CastInstantSkill(lua_State* lstate);   // 施放即时技能
	static int TerminateCombat(lua_State* lstate);    // 强制结束战斗
	static int WaitSelectSkill(lua_State* lstate);    // 等待选择技能
	static int WaitSelectPet(lua_State* lstate);      // 等待选择宠物
    static int Shake(lua_State* lstate);              // 场景震动
    static int GetUnitProp(lua_State* lstate);        // 获取指定对象的属性值
    static int GetUnitType(lua_State* lstate);        // 获取指定对象的类型

	static int GetTotalRoundCount(lua_State* lstate); // 获取战场总的回合数
	static int GetMobList(lua_State* lstate);         // 获取战场有哪些怪物
};

};

#endif // __GAME_MODULE_COMBAT_LUA_API_H__
