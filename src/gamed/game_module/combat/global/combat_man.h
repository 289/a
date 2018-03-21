#ifndef __GAME_MODULE_COMBAT_MAN_H__
#define __GAME_MODULE_COMBAT_MAN_H__

#include "obj_man.h"
#include "combat_mq.h"
#include "combat_def.h"
#include "combat_types.h"
#include "combat_param.h"
#include "shared/base/singleton.h"
#include "shared/base/time_entry.h"

namespace combat
{

class ExtendMsgQueue;

class Object;
class WorldBoss;
class Combat;
class CombatUnit;
class CombatPVE;
class CombatPVP;
class CombatNpc;
class CombatPlayer;
class CombatMan : public shared::Singleton<CombatMan>
{
private:
	class Insertor
	{
	public:
		template <typename T>
		static int push_back(std::vector<T*>& list, T* obj)
		{
			list.push_back(obj);
			return 0;
		}
		static int push_back(std::vector<CombatPlayer*>& list, CombatPlayer* obj)
		{
			return 0;
		}
		static int push_back(std::vector<CombatNpc*>& list, CombatNpc* obj)
		{
			return 0;
		}
	};
    
	ObjectMan<CombatNpc, COMBAT_TICK_PER_SEC, Insertor>    combat_npc_man_;
	ObjectMan<CombatPlayer, COMBAT_TICK_PER_SEC, Insertor> combat_player_man_;
	ObjectMan<CombatPVE, COMBAT_TICK_PER_SEC, Insertor>    pve_combat_man_;
	ObjectMan<CombatPVP, COMBAT_TICK_PER_SEC, Insertor>    pvp_combat_man_;
	ObjectMan<WorldBoss, COMBAT_TICK_PER_SEC, Insertor>    world_boss_man_;

	MsgQueueList msg_queue_;
	int32_t tick_counter_;

public:
	CombatMan();
	~CombatMan();

	static CombatMan* GetInstance()
	{
		static CombatMan man;
		return &man;
	}

	bool Initialize(std::vector<int>& mapid_vec, std::string& combat_script_path, const std::vector<cls_prop_sync_rule>& list);
	void Release();
	void HeartBeat();

	///
	/// 对象池操作函数
	///
	CombatPVE* AllocPVECombat();
	CombatPVP* AllocPVPCombat();
	CombatNpc* AllocNPC();
	CombatPlayer* AllocPlayer(RoleID role_id);
	WorldBoss* AllocWorldBoss(ObjectID obj_id);

	void FreePVECombat(CombatPVE *);
	void FreePVPCombat(CombatPVP *);
	void FreeNPC(CombatNpc *);
	void FreePlayer(CombatPlayer *);
	void FreeWorldBoss(WorldBoss*);

	void InsertPVECombatToMan(CombatPVE *);
	void InsertPVPCombatToMan(CombatPVP *);
	void InsertNPCToMan(CombatNpc *);
	void InsertPlayerToMan(CombatPlayer *);
	void InsertWorldBossToMan(WorldBoss *);

	void RemovePVECombatFromMan(CombatPVE *);
	void RemovePVPCombatFromMan(CombatPVP *);
	void RemoveNPCFromMan(CombatNpc *);
	void RemovePlayerFromMan(CombatPlayer *);
	void RemoveWorldBossFromMan(WorldBoss *);

	size_t GetObjIndex(const CombatPVE*) const;
	size_t GetObjIndex(const CombatPVP*) const;
	size_t GetObjIndex(const CombatNpc*) const;
	size_t GetObjIndex(const CombatPlayer*) const;
	size_t GetObjIndex(const WorldBoss*) const;

	///
	/// 查询函数，复杂度为O(1)
	///
	CombatUnit* QueryCombatUnit(UnitID unit_id);
	CombatUnit* QueryCombatUnit(const XID& xid);
	Combat* QueryCombat(ObjectID combat_id);
	Combat* QueryCombat(const XID& xid);
	WorldBoss* QueryBoss(UnitID unit_id);
	WorldBoss* QueryBoss(const XID& xid);
	Object* QueryObject(ObjectID object_id);

	///
	/// 根据对象在大世界的ID来查询，复杂度为0(lgN)，查询效率相对Query开头的低
    /// 目前只支持查询玩家和世界BOSS
    /// Find开头的查询会占用对象池的锁资源，所以要慎用，优先使用Query查询函数。
	///
	CombatPlayer* FindPlayer(RoleID roleid);  // roleid为玩家在大世界的角色ID
	WorldBoss* FindWorldBoss(int32_t bossid); // bossid为BOSS在大世界的ID

	///
	/// 发送消息
	///
	void SendMSG(const MSG& msg, size_t latency);
	void SendMSG(const ObjectVec& list, const MSG& msg, size_t latency);
	void SendMSG(const XID* first, const XID* last, const MSG& msg, size_t latency);
	void DispatchMSG(const MSG& msg);
	void DispatchMSG(Object* object, const MSG& msg);

private:
	///
	/// 查询对象接口,通过本函数组获得的对象未加锁
	/// 禁止外面调用,外面使用QueryCombat和QueryCombatUnit
	///
	CombatPVE* QueryPVECombat(ObjectID combat_id);
	CombatPVP* QueryPVPCombat(ObjectID combat_id);
	CombatNpc* QueryNPC(UnitID unit_id);
	CombatPlayer* QueryPlayer(ObjectID unit_id);
	WorldBoss* QueryWorldBoss(ObjectID boss_id);

	void Trace() const;
};

#define s_pCombatMan combat::CombatMan::GetInstance()

}; // namespace combat

#endif //__GAME_MODULE_COMBAT_MAN_H__
