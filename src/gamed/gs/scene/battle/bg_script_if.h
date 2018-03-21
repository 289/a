#ifndef GAMED_GS_SCENE_BATTLE_BG_SCRIPT_IF_H_
#define GAMED_GS_SCENE_BATTLE_BG_SCRIPT_IF_H_

#include <stdint.h>
#include <string>

#include "shared/net/packet/bytebuffer.h"
#include "gs/global/game_types.h"


namespace gamed {

class GameLuaEntity;

/**
 * @brief BGScriptIf
 *  （1）作为BGWorldManImp的成员变量，和BGWorldManImp绑定在一起
 */
class BGScriptIf
{
public:
	BGScriptIf();
	~BGScriptIf();

    static void InitScriptSys(const char* script_dir);

    /**
     * @brief Init 
     * @param script_id和战场模板id一致
     */
	bool Init(const XID& world_xid, int32_t script_id, const char* script_dir);

	/**
	 * @brief BattleGroundStart 
	 *  （1）战场开始
	 */
	void BattleGroundStart();

	/**
	 * @brief PlayerEnter 
	 * @param role_id: 玩家的角色id
	 */
	void PlayerEnter(int64_t role_id);

	/**
	 * @brief MonsterKilled 
	 * @param monster_tid: 死亡怪物的模板id
     *        count: 死亡怪物的数量
	 */
	void MonsterDead(int32_t monster_tid, int32_t count);
	
	/**
	 * @brief CounterChange 
	 * @param index: 计数器的索引
	 * @param cur_value: 计数器改变后的当前值
	 */
	void CounterChange(int32_t index, int32_t cur_value);

	/**
	 * @brief ClockTimeUp 
	 * @param index: 计时器的索引
	 */
	void ClockTimeUp(int32_t index);

    /**
     * @brief PlayerGatherMine 
     * @param roleid: 玩家id
     * @param mine_tid：被采集的矿的模板id
     */
    void PlayerGatherMine(int64_t roleid, int32_t mine_tid);

    /**
     * @brief ReachDestination 
     * @param elem_id: 地图元素
     * @param pos: 到达的目标点
     */
    void ReachDestination(int32_t elem_id, const A2DVECTOR& pos);

public:
    enum ScriptIFType
    {
        BG_START,
        PLAYER_ENTER,
        MONSTER_DEAD,
        COUNTER_CHANGE,
        CLOCK_TIMEUP,
        PLAYER_GATHER_MINE,
        REACH_DESTINATION,
        SIFT_MAX
    };

private:
    bool InitLuaScript(const std::string& script_file);
	inline bool has_script() const;

private:
	XID     world_xid_;
	int32_t script_id_;
	std::string script_file_;

    // script function name
    shared::net::FixedArray<std::string, SIFT_MAX> script_func_;

    // lua state
    GameLuaEntity* lua_ent_;
};

///
/// inline func
///
inline bool BGScriptIf::has_script() const
{
	return !script_file_.empty();
}

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_SCRIPT_IF_H_
