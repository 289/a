#ifndef GAMED_GS_SUBSYS_PLAYER_COMBAT_H_
#define GAMED_GS_SUBSYS_PLAYER_COMBAT_H_

#include "gs/player/player_subsys.h"

namespace combat
{
class CombatPlayer;
struct PlayerCMD;
struct player_data;
};

namespace gamed
{

///
/// 战斗子系统
///

class PlayerCombat : public PlayerSubSystem
{
private:
	struct ItemEntry
	{
		int32_t item_id;
		int32_t item_count;
		NESTED_DEFINE(item_id, item_count);
	};

	struct MobKilled
	{
		int32_t mob_id;     //击杀怪物的ID
		int32_t mob_killed; //击杀怪物个数
		NESTED_DEFINE(mob_id, mob_killed);
	};

	struct WorldBossData
	{
		WorldBossData()
			: monster_obj_id(0), combat_scene_id(0), mob_group_id(0)
		{ }

		void clear() 
		{
			monster_obj_id  = 0;
			combat_scene_id = 0;
			mob_group_id    = 0;
		}

		int64_t monster_obj_id;
		int32_t combat_scene_id;
		int32_t mob_group_id;
	};

    enum COMBAT_TYPE
    {
        COMBAT_TYPE_INVALID,
        COMBAT_TYPE_PVE,
        COMBAT_TYPE_PVP,
    };

	typedef std::vector<ItemEntry> ItemVec;
	typedef std::vector<MobKilled> MobKilledVec;
    typedef std::vector<RoleID> PlayerKilledVec;

private:
	int32_t unit_id_;	                  // 玩家在战场的ID
	int32_t combat_id_;                   // 玩家所在战场ID
    int8_t  combat_type_;                 // 当前战斗的类型
	int8_t  combat_result_;               // 战斗结果：输赢
	int32_t combat_remain_hp_;            // 战斗剩余血量
    int32_t combat_pet_power_;            // 宠物能量值
	int64_t combat_award_exp_;            // 战斗奖励：经验
	int64_t combat_award_money_;          // 战斗奖励：金钱
	ItemVec combat_award_items_drop_;     // 战斗奖励：普通掉落和全局掉落物品
	ItemVec combat_award_items_lottery_;  // 战斗奖励：特殊掉落物品--抽奖物品
	MobKilledVec mob_killed_vec_;         // 玩家击杀怪物的列表
    PlayerKilledVec player_killed_vec_;   // 玩家击杀玩家的列表
	int  time_waiting_mob_response_;      // 等待怪物响应的超时时间
	bool is_waiting_mob_response_;        // 正在等待怪物响应
	bool is_waiting_combat_result_;       // 正在等待DB返回战斗结果
	int64_t monster_object_id_;           // 正在战斗的大世界怪物id
	int32_t combat_query_count_;          // 共享血量战斗查询数量（向周围发了多少个消息出去）
	int32_t combat_query_timeout_;        // 共享血量战斗查询超时时间
	WorldBossData boss_data_;             // 世界boss数据保存一份在身上
    int32_t combat_pvp_type_;             // PVP战斗的类型：决斗、竞技场等等，对应playerdef::CombatPVPType


public:
	PlayerCombat(Player& player);
	~PlayerCombat();

	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();
	virtual void OnHeartbeat(time_t cur_time);

	bool SaveToDB(common::PlayerCombatData* pData);
	bool LoadFromDB(const common::PlayerCombatData& data);
	void PlayerGetCombatData();
	bool TerminateCombat();
	bool StartPVPCombat(int32_t combat_scene_id, playerdef::CombatPVPType type, playerdef::StartPVPResult& result);
	bool FirstJoinPVPCombat(int32_t combat_id);


protected:
	void CMDHandler_JoinCombat(const C2G::CombatPlayerJoin&);
	void CMDHandler_TriggerCombat(const C2G::CombatPlayerTrigger&);
	void CMDHandler_SelectSkill(const C2G::CombatSelectSkill&);
	void CMDHandler_PetAttack(const C2G::CombatPetAttack&);

	int  MSGHandler_ObjTriggerCombat(const MSG&);
	int  MSGHandler_SysTriggerCombat(const MSG&);
	int  MSGHandler_PlayerTriggerCombatRe(const MSG&);
	int  MSGHandler_CombatStart(const MSG&);
	int  MSGHandler_CombatPVEEnd(const MSG&);
	int  MSGHandler_CombatPVPEnd(const MSG&);
	int  MSGHandler_CombatPVEResult(const MSG&);
	int  MSGHandler_CombatPVPResult(const MSG&);
	int  MSGHandler_SomeoneHateYou(const MSG&); // 有object已经watch该player，即将发动攻击等ai动作
	int  MSGHandler_CompanionEnterCombat(const MSG&);
	int  MSGHandler_QueryTeamCombat(const MSG&);
	int  MSGHandler_QueryTeamCombatRe(const MSG&);


private:
	void InitCombatPlayer(combat::CombatPlayer* player);
	bool CreatePVECombat(int32_t combat_scene_id, int32_t mob_group_id, int32_t monster_obj_id, bool is_world_boss=false, int32_t taskid=0, int32_t challenge_id = 0);
	bool CreatePVPCombat(int32_t combat_scene_id, int32_t pvp_type);
	void OnEnterCombat();
	void OnLeaveCombat();
	bool InCombat() const;
	void ClrCombatInfo();

	bool IsWaitMobResponse() const;
	void SetWaitMobResponse(int time);
	void ClrWaitMobResponse();

	void BuildPlayerData(combat::player_data& data);
	bool JoinCombat(int32_t combat_id, int64_t role_to_join);
	void CheckQueryTeamCombat();
	void SendObjTriggerCombatRe(const XID& target, bool is_success, int32_t interval);
    void SyncPlayerCombatData();
    void HandleCombatPVPEnd(RoleID creator, int32_t pvp_type);
    void SyncInfoToWorldBoss(int32_t monster_obj_id);
};

///
/// inline func
///
inline bool PlayerCombat::InCombat() const
{
	return combat_id_ > 0;
}

inline bool PlayerCombat::IsWaitMobResponse() const
{
	return is_waiting_mob_response_;
}

inline void PlayerCombat::SetWaitMobResponse(int time)
{
	is_waiting_mob_response_   = true;
	time_waiting_mob_response_ = time;
}

inline void PlayerCombat::ClrWaitMobResponse()
{
	is_waiting_mob_response_   = false;
	time_waiting_mob_response_ = 0;
}

}; // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_COMBAT_H_
