#include "combat_if.h"
#include "combat.h"
#include "combat_unit.h"
#include "combat_npc.h"
#include "combat_player.h"
#include "world_boss.h"
#include "combat_man.h"

namespace combat
{

void SetCombatCMDSenderCallBack(const CombatSenderCallBack& cb)
{
	Combat::SetCombatCMDSenderCallBack(cb);
}

void SetCombatPVEResultCallBack(const CombatPVEResultCallBack& cb)
{
	Combat::SetCombatPVEResultCallBack(cb);
}

void SetCombatPVPResultCallBack(const CombatPVPResultCallBack& cb)
{
	Combat::SetCombatPVPResultCallBack(cb);
}

void SetCombatStartCallBack(const CombatStartCallBack& cb)
{
	Combat::SetCombatStartCallBack(cb);
}

void SetCombatPVEEndCallBack(const CombatPVEEndCallBack& cb)
{
	Combat::SetCombatPVEEndCallBack(cb);
}

void SetCombatPVPEndCallBack(const CombatPVPEndCallBack& cb)
{
	Combat::SetCombatPVPEndCallBack(cb);
}

void SetWorldBossStatusCallBack(const WorldBossStatusCallBack& cb)
{
	Combat::SetWorldBossStatusCallBack(cb);
}

bool Initialize(std::vector<int>& mapid_vec, std::string& npc_policy_path, const std::vector<cls_prop_sync_rule>& list)
{
	return s_pCombatMan->Initialize(mapid_vec, npc_policy_path, list);
}

void HeartBeat()
{
	s_pCombatMan->HeartBeat();
}

void Release()
{
	s_pCombatMan->Release();
}

Combat* QueryCombat(ObjectID combat_id)
{
	return s_pCombatMan->QueryCombat(combat_id);
}

int QueryCombatType(CombatID combat_id)
{
    Combat* combat = s_pCombatMan->QueryCombat(combat_id);
    if (combat)
        return combat->GetType();
    return 0;
}

CombatUnit* QueryCombatUnit(UnitID unit_id)
{
	return s_pCombatMan->QueryCombatUnit(unit_id);
}

CombatPlayer* FindPlayer(RoleID roleid)
{
    return s_pCombatMan->FindPlayer(roleid);
}

bool CreatePVECombat(const pve_combat_param& param, ObjectID& combat_id, UnitID& unit_id)
{
    if (s_pCombatMan->FindPlayer(param.creator))
    {
        //创建战斗的玩家已经在战场中了。。。
        return false;
    }

    //首先处理是不是世界BOSS战斗，如果是则需要检查BOSS是否死亡。
    //这里的检查是不加锁的，后面进一步的检查是加锁的。
    //尽管这里做了检查，但是后面的检查还是有可能失败的，只不过失败的概率大大降低了。
	if (param.is_world_boss)
	{
		int32_t world_boss_id = param.world_monster_id;
		WorldBoss* world_boss = s_pCombatMan->FindWorldBoss(world_boss_id);
		if (world_boss && !world_boss->IsActived())
        {
            //世界BOSS被释放了。。。
            return false;
        }

        if (world_boss && world_boss->IsWorldBossDead())
        {
            //世界BOSS死亡了。。。
            return false;
        }
    }

	CombatPVE* combat = s_pCombatMan->AllocPVECombat();
	if (!combat)
	{
		return false;
	}

	//设置战场信息
	combat->SetCreator(param.creator);
    combat->SetTaskID(param.task_id);
    combat->SetChallengeID(param.challenge_id);
	combat->SetInitPlayerCount(param.init_player_count + param.team_npc_vec.size());
	combat->SetMobGroupId(param.mob_group_id);
	combat->SetWorldMonsterID(param.world_monster_id);
	combat->SetWorldBossCombat(param.is_world_boss);

	if (!combat->Init(param.world_id, param.combat_scene_id))
	{
		__PRINTF("初始化战场失败!!!");
		s_pCombatMan->FreePVECombat(combat);
		combat->Unlock();
		return false;
	}

	CombatPlayer* player = s_pCombatMan->AllocPlayer(param.playerdata.roleid);
    if (!player)
	{
		s_pCombatMan->FreePVECombat(combat);
		combat->Unlock();
		return false;
	}

#define CLEANUP() \
	{ \
		s_pCombatMan->FreePlayer(player); \
		s_pCombatMan->FreePVECombat(combat); \
		player->Unlock(); \
		combat->Unlock(); \
	}

	//设置玩家信息
	player->Initialize();
	player->SetCombat(combat);
	if (!player->Load(param.playerdata))
	{
		CLEANUP();
		return false;
	}

	//加入战场
	if (!combat->AddCombatUnit(player))
	{
		CLEANUP();
		return false;
	}

	if (!player->LoadPetData(param.playerdata))
	{
		CLEANUP();
		return false;
	}
#undef CLEANUP

	/// 注意：
	/// 占位战斗对象一旦加入战场成功，则必须同时也放入对象池，否则战场释放的时候将出问题。
	/// 放入对象池后，后面再有创建战斗失败返回时，都无需再释放这个占位战斗对象了，因为战场销毁时会统一释放战场内的占位战斗对象。
	s_pCombatMan->InsertPlayerToMan(player);

	//添加组队NPC
	for (size_t i = 0; i < param.team_npc_vec.size(); ++ i)
	{
		const pve_combat_param::team_npc& npc = param.team_npc_vec[i];
		if (!combat->AddTeamNpc(npc.id, npc.pos, player))
		{
			player->Unlock();
			s_pCombatMan->FreePVECombat(combat);
			combat->Unlock();
			return false;
		}
	}

	//判断是否是世界BOSS
	if (param.is_world_boss)
	{
		int32_t world_boss_id = param.world_monster_id;
		WorldBoss* world_boss = s_pCombatMan->FindWorldBoss(world_boss_id);
		if (world_boss)
		{
			///
			/// 已经有人在打这个BOSS了
			///
			if (!world_boss->IsActived())
			{
				//运气太背，正好释放了...
				player->Unlock();
				s_pCombatMan->FreePVECombat(combat);
				combat->Unlock();
				return false;
			}

			shared::MutexLockGuard keeper(world_boss->GetMutex());

			if (!world_boss->IsActived())
			{
				//世界BOSS被释放了...
				player->Unlock();
				s_pCombatMan->FreePVECombat(combat);
				combat->Unlock();
				return false;
			}

			if (world_boss->IsWorldBossDead())
			{
				//世界BOSS已经死了
				player->Unlock();
				s_pCombatMan->FreePVECombat(combat);
				combat->Unlock();
				return false;
			}

			std::vector<CombatUnit*> mob_list;
			combat->GetEnemy(player->GetXID(), mob_list);
			for (size_t i = 0; i < mob_list.size(); ++ i)
			{
				//BOSS怪物共享血量
				CombatUnit* unit = mob_list[i];
				if (!unit) continue;
				CombatNpc* boss = dynamic_cast<CombatNpc*>(unit);
				int pos = boss->GetPos();
				if (!world_boss->CheckBoss(pos, boss->GetNpcID()))
				{
					__PRINTF("世界BOSS身上的怪物组不是怪物固定怪物组!!!");

					player->Unlock();
					s_pCombatMan->FreePVECombat(combat);
					combat->Unlock();
					return false;
				}

				boss->SyncHP(world_boss->GetBossHP(pos));
			}

			world_boss->RegisterCombat(combat->GetID());
            world_boss->RegisterPlayer(combat->GetID(), player->GetID(), player->GetRoleID());
			world_boss->SetWorldBossObjectID(param.world_monster_id);
		}
		else
		{
			///
			/// 第一次打这个BOSS
			///
			world_boss = s_pCombatMan->AllocWorldBoss(param.world_monster_id);
			if (!world_boss)
			{
				__PRINTF("战斗系统的世界BOSS对象耗尽!!!");

				player->Unlock();
				s_pCombatMan->FreePVECombat(combat);
				combat->Unlock();
				return (false);
			}

			//初始化世界BOSS对象
			std::vector<CombatUnit*> mob_list;
			combat->GetEnemy(player->GetXID(), mob_list);
			for (size_t i = 0; i < mob_list.size(); ++ i)
			{
				CombatUnit* unit = mob_list[i];
				if (!unit) continue;
				CombatNpc* boss = dynamic_cast<CombatNpc*>(unit);
				world_boss->AddBoss(boss->GetNpcID(), boss->GetPos(), boss->GetHP());
			}

			world_boss->RegisterCombat(combat->GetID());
            world_boss->RegisterPlayer(combat->GetID(), player->GetID(), player->GetRoleID());
			world_boss->SetWorldBossObjectID(param.world_monster_id);

			world_boss->Unlock();

			s_pCombatMan->InsertWorldBossToMan(world_boss);
		}

		combat->SetWorldBossID(world_boss->GetID());
	}

	//开始战斗
	if (!combat->StartCombat())
    {
        player->Unlock();
        s_pCombatMan->FreePVECombat(combat);
        combat->Unlock();
        return false;
    }

	unit_id = player->GetID();
	combat_id = combat->GetCombatID();

	//把战场对象加入对象池
	s_pCombatMan->InsertPVECombatToMan(combat);

	player->Unlock();
	combat->Unlock();

	return true;
}

bool CreatePVPCombat(const pvp_combat_param& param, ObjectID& combat_id, UnitID& unit_id)
{
    if (s_pCombatMan->FindPlayer(param.creator))
    {
        //创建战斗的玩家已经在战场中了。。。
        return false;
    }

	CombatPVP* combat = s_pCombatMan->AllocPVPCombat();
	if (!combat)
	{
		return false;
	}

	//设置战场信息
	combat->SetCreator(param.creator);
    combat->SetCombatFlag(param.combat_flag);
	if (!combat->Init(param.world_id, param.combat_scene_id))
	{
		__PRINTF("初始化战场失败!!!");
		s_pCombatMan->FreePVPCombat(combat);
		combat->Unlock();
		return false;
	}

	CombatPlayer* player = s_pCombatMan->AllocPlayer(param.playerdata.roleid);
    if (!player)
	{
		s_pCombatMan->FreePVPCombat(combat);
		combat->Unlock();
		return false;
    }

#define CLEANUP() \
	{ \
		s_pCombatMan->FreePlayer(player); \
		s_pCombatMan->FreePVPCombat(combat); \
		player->Unlock(); \
		combat->Unlock(); \
	}

	//设置玩家信息
	player->Initialize();
	player->SetCombat(combat);
	if (!player->Load(param.playerdata))
	{
		CLEANUP();
		return false;
	}

	//加入战场
	if (!combat->AddCombatUnit(player))
	{
		CLEANUP();
		return false;
	}

	if (!player->LoadPetData(param.playerdata))
	{
		CLEANUP();
		return false;
	}
#undef CLEANUP

	/// 注意：
	/// 占位战斗对象一旦加入战场成功，则必须同时也放入对象池，否则战场释放的时候将出问题。
	/// 放入对象池后，后面再有创建战斗失败返回时，都无需再释放这个占位战斗对象了，因为战场销毁时会统一释放战场内的占位战斗对象。
	s_pCombatMan->InsertPlayerToMan(player);

	//添加组队NPC
	for (size_t i = 0; i < param.team_npc_vec.size(); ++ i)
	{
		const pvp_combat_param::team_npc& npc = param.team_npc_vec[i];
		if (!combat->AddTeamNpc(npc.id, npc.pos, player))
		{
			player->Unlock();
			s_pCombatMan->FreePVPCombat(combat);
			combat->Unlock();
			return false;
		}
	}

	//开始战斗
	combat->StartCombat();

	unit_id = player->GetID();
	combat_id = combat->GetCombatID();

	//把战场对象加入对象池
	s_pCombatMan->InsertPVPCombatToMan(combat);

	player->Unlock();
	combat->Unlock();

    return true;
}

bool DispatchCmd(const PlayerCMD& cmd, ObjectID combat_id, UnitID unit_id)
{
	Combat* combat = s_pCombatMan->QueryCombat(combat_id);
	if (!combat)
	{
		return false;
	}

	shared::MutexLockGuard keeper(combat->GetMutex());
	if (!combat->IsActived())
	{
		return false;
	}
	
	//这里player偷懒不加锁了
	CombatPlayer* player = combat->QueryPlayer(cmd.roleid);
	if (!player || player->GetID() != unit_id)
	{
		return false;
	}

	return combat->HandleCMD(cmd);
}

bool JoinCombat(RoleID roleid, const player_data& playerdata, RoleID role_to_join, ObjectID combat_id, UnitID& unit_id)
{
    if (s_pCombatMan->FindPlayer(roleid))
    {
        //准备加入战斗的玩家已经在战场中了。。。
        return false;
    }

	Combat* combat = s_pCombatMan->QueryCombat(combat_id);
	if (!combat)
	{
		__PRINTF("战场%d不存在.", combat_id);
		return false;
	}
    if (combat->GetChallengeID() != 0)
    {
		__PRINTF("战场%d 界面BOSS挑战%d 不允许多人进入", combat_id, combat->GetChallengeID());
		return false;
    }

	if (combat->QueryPlayer(roleid))
	{
		__PRINTF("玩家%ld已经在战斗中.", playerdata.roleid);
		return false;
	}

	shared::MutexLockGuard keeper(combat->GetMutex());

	if (!combat->IsActived())
	{
		return false;
	}
	
	if (!combat->CanJoinCombat())
	{
		return false;
	}

	//创建玩家战斗对象
	CombatPlayer* player = s_pCombatMan->AllocPlayer(playerdata.roleid);
	if (!player)
	{
		return false;
	}

	//初始化玩家战斗对象
	player->Initialize();
	player->SetCombat(combat);
	if (!player->Load(playerdata))
	{
		s_pCombatMan->FreePlayer(player);
		player->Unlock();
		return false;
	}

	//加入战场
	if (!combat->AddCombatUnit(player, role_to_join))
	{
		s_pCombatMan->FreePlayer(player);
		player->Unlock();
		return false;
	}

	if (!player->LoadPetData(playerdata))
	{
		s_pCombatMan->FreePlayer(player);
		player->Unlock();
		return false;
	}

	//检查是否是世界BOSS战场
	if (combat->IsWorldBossCombat())
	{
		WorldBoss* world_boss = s_pCombatMan->QueryBoss(dynamic_cast<CombatPVE*>(combat)->GetWorldBossID());
		if (world_boss)
		{
			shared::MutexLockGuard keeper(world_boss->GetMutex());
			if (!world_boss->IsActived())
			{
				assert(false && "战场还在世界BOSS对象不应该被释放啊!!!");
				s_pCombatMan->FreePlayer(player);
				player->Unlock();
				return false;
			}
            world_boss->RegisterPlayer(combat_id, player->GetID(), playerdata.roleid);
		}
        else
        {
            s_pCombatMan->FreePlayer(player);
            player->Unlock();
            return false;
        }
    }

	//插入玩家对象池
	s_pCombatMan->InsertPlayerToMan(player);

	//玩家成功加入
	player->OnPlayerJoin();

	unit_id = player->GetID();

	player->Unlock();

	return true;
}

};
