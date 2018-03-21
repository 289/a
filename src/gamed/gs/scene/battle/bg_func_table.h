#ifndef GAMED_GS_SCENE_BATTLE_BG_FUNC_TABLE_H_
#define GAMED_GS_SCENE_BATTLE_BG_FUNC_TABLE_H_

#include "shared/lua/lua_engine.h"


namespace gamed {
namespace BGScript {

	enum ReturnCode
	{
		RC_FAILURE = -1,
		RC_SUCCESS = 1,
	};
	
	/*以下函数所需参数见cpp文件*/

	///
	/// 地图元素
	///
	int ActivateMapElem(lua_State* state);
	int DeactivateMapElem(lua_State* state);

	///
	/// 任务相关
	///
	int DeliverTask(lua_State* state);
    int DeliverTaskToAll(lua_State* state);

	///
	/// 计数器
	///
	int IncreaseCounter(lua_State* state);
	int DecreaseCounter(lua_State* state);
	int AssignmentCounter(lua_State* state);
    int GetCounter(lua_State* state);
    int LockCounter(lua_State* state);
    int UnlockCounter(lua_State* state);
    int IsCounterLocked(lua_State* state);

	///
	/// 计时器
	///
	int ActivateClock(lua_State* state);
	int DeactivateClock(lua_State* state);

	///
	/// 关闭战场
	///
	int CloseBattleGround(lua_State* state);

    ///
    /// 地图提示信息
    ///
    int ShowPromptMessage(lua_State* state);
    int ShowCountDown(lua_State* state);

    ///
    /// 定点地图元素瞬移（npc、怪、矿）
    ///
    int SpotMapElemTeleport(lua_State* state);

    ///
    /// 定点怪移动到某个点
    ///
    int SpotMonsterMoveTo(lua_State* state);
    int SetSpotMonsterSpeed(lua_State* state);

	///
	/// function table
	///
	extern luabind::CFunction bg_script_func_table[];
	extern int func_table_size;

} // namespace BGScript
} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_FUNC_TABLE_H_
