#ifndef GAMED_GS_OBJ_WORLD_BOSS_H_
#define GAMED_GS_OBJ_WORLD_BOSS_H_

#include "monster_imp.h"

#include "gs/global/msg_pack_def.h"


namespace combat {
    struct WorldBossCombatStatus;
} // namespace combat


namespace gamed {

class WorldBossImp : public MonsterImp
{
    static const size_t kMaxRecordPlayerCount = 500; // 最多记录多少个玩家的伤害记录
public:
	WorldBossImp(Npc& npc);
	virtual ~WorldBossImp();

	virtual bool OnInit();
	virtual int  OnMessageHandler(const MSG& msg);
    virtual void OnHasInsertToWorld();

protected:
	virtual void HandleCombatEnd(const msg_combat_end& msg_param);
	virtual bool CheckStateToCombat();

private:
	void HandleBossCombatResult(const MSG& msg);
    void HandleSyncPlayerInfo(const MSG& msg);
    void RefreshDamageList(const combat::WorldBossCombatStatus& result);
    void CalcCombatResult();
    void RetainMasterID(int32_t master_id);
    void SyncGlobalRecord(int32_t master_id);
    void NotifyWorldBossDead();
    void TryResetGlobalData(int32_t master_id);

private:
    struct DamageResult
    {
        int64_t damage;
        msgpack_wb_player_info info;
    };

private:
    bool   reset_global_record_;
    bool   is_in_worldboss_map_; // 是否在世界BOSS的战场里，在战场里才同步伤害排行
    time_t timestamp_of_born_;   // 出生的时间戳，用于globaldata全局数据对比新旧

    typedef std::map<RoleID, msgpack_wb_player_info> PlayerInfoMap;
    PlayerInfoMap    player_info_map_;

    typedef std::map<RoleID, int64_t/*damage*/> DamageInfoMap;
    DamageInfoMap    damage_info_map_;

    typedef std::map<RoleID, DamageResult> DamageResultMap;
    DamageResultMap  damage_res_map_; // 保存临时结果

    typedef std::set<int32_t/*masterid*/> MasterSet;
    MasterSet        master_set_;

    typedef std::multimap<int64_t/*damage*/, RoleID, std::greater<int64_t> > DamageInfoMultimap;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_WORLD_BOSS_H_
