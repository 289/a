#ifndef __GAMED_MODULE_COMBAT_WORLD_BOSS_H__
#define __GAMED_MODULE_COMBAT_WORLD_BOSS_H__

#include <map>
#include <vector>
#include "object.h"
#include "shared/base/mutex.h"


namespace combat {

/**
 * @class WorldBoss
 * @brief 大世界BOSS在战斗系统对应的对象
 * @brief 一个WorldBoss对象对应大世界的一个BOSS,
 * @brief 注意区分2个概念：WorldBoss和CombatBoss, 
 * @brief WorldBoss表示大世界BOSS, CombatBoss表示战斗中BOSS, 一个WorldBoss可能对应多个CombatBoss.
 * @brief WorldBoss和CombatBoss之间的通信：WorldBoss可以对战场加锁，从而拿到CombatBoss，但是CombatBoss只能通过消息和WorldBoss交互。
 */
class WorldBoss : public Object
{
public:
	WorldBoss();
	virtual ~WorldBoss();

	virtual void HeartBeat();
	virtual void MessageHandler(const MSG& msg);
	virtual void Trace() const;

	void Clear();

	///
	/// 世界BOSS相关
	///
	bool    IsWorldBossDead() const;
	void    SetWorldBossObjectID(int32_t id);
	int32_t GetWorldBossObjectID() const;

	///
	/// 注册玩家和战场
	///
	void    RegisterCombat(ObjectID combat_id);
    void    RegisterPlayer(CombatID combat_id, UnitID unit_id, RoleID role_id);

	///
	/// 战斗BOSS相关
	///
	void    AddBoss(TemplID boss_tid, int pos, int32_t hp);
	bool    CheckBoss(int pos) const;
	bool    CheckBoss(int pos, TemplID boss_tid) const;
	int32_t GetBossHP(int pos) const;


private:
	void DoHeal(ObjectID combat_id, int boss_pos, int life);
	void DoDamage(ObjectID combat_id, int boss_pos, int damage, UnitID attacker);
	void SyncCombatBossHP();
	void MarkCombatEnd(ObjectID combat_id, int result);
	bool TestCombatEnd() const;
	bool TestWorldBossDead() const;
	void DoCombatEnd();
    void SyncWorldBossStatus(ObjectID combat_id);


private:
	struct CombatNode
	{
		ObjectID combat_id;
		bool     running;
		int      result;
	};

	struct BossNode
	{
		TemplID tid;
		int32_t hp;
		int8_t  hp_change;
		int8_t  pos;
	};

	struct boss_finder
	{
		int pos;
		boss_finder(int _pos): pos(_pos) {}
		bool operator() (const BossNode& boss) const
		{
			return boss.pos == pos;
		}
	};

	struct PlayerNode
	{
        PlayerNode() 
            : roleid(0), 
              damage(0) 
        { }

		RoleID roleid;
		Damage damage;
	};

	typedef std::vector<BossNode>   BossVec;
	typedef std::vector<CombatNode> CombatVec;
	typedef std::map<UnitID, PlayerNode>  DamageMap;
    typedef std::map<CombatID, DamageMap> PlayerMap;

	BossVec      combat_boss_list_;     // BOSS战中有哪些BOSS
	CombatVec    combat_list_;          // 世界BOSS共开了几场战斗
	PlayerMap    player_map_;           // 有哪些玩家在打这个BOSS
	int32_t      world_boss_object_id_; // 世界BOSS在大世界的对象ID
	bool         some_combat_end_;      // 距离上次心跳是否有战斗结束
	bool         boss_hp_change_;       // 距离上次心跳是否有BOSS血量变化
	bool         is_combating_;         // 世界BOSS正在战斗中
};

///
/// inline func
///
inline void WorldBoss::SetWorldBossObjectID(int32_t id)
{
	if (!world_boss_object_id_)
	{
		world_boss_object_id_ = id;
	}
	else
	{
		bool rst = (world_boss_object_id_ == id);
		assert(rst);
	}
}

inline int32_t WorldBoss::GetWorldBossObjectID() const
{
	return world_boss_object_id_;
}

} // namespace combat

#endif // __GAMED_MODULE_COMBAT_WORLD_BOSS_H__
