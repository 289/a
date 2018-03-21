#ifndef GAMED_GS_SCENE_INSTANCE_INS_SCRIPT_IF_H_
#define GAMED_GS_SCENE_INSTANCE_INS_SCRIPT_IF_H_

#include <stdint.h>
#include <string>

#include "shared/net/packet/bytebuffer.h"
#include "gs/global/game_types.h"


namespace gamed {

class GameLuaEntity;

/**
 * @brief InsScriptIf
 *  （1）作为InsWorldManImp的成员变量，和InsWorldManImp绑定在一起
 */
class InsScriptIf
{
public:
	InsScriptIf();
	~InsScriptIf();

    static void InitScriptSys(const char* script_dir);

    /**
     * @brief Init 
     * @param script_id和副本模板id一致
     */
	bool Init(const XID& world_xid, int32_t script_id, const char* script_dir);

	/**
	 * @brief InstanceStart 
	 *  （1）副本开始
	 */
	void InstanceStart();

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
        INS_START,
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
inline bool InsScriptIf::has_script() const
{
	return !script_file_.empty();
}

} // namespace gamed

#endif // GAMED_GS_SCENE_INSTANCE_INS_SCRIPT_IF_H_
