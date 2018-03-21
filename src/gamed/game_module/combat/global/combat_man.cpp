#include <limits> // std::numeric_limits

#include "combat.h"
#include "combat_mq.h"
#include "combat_man.h"
#include "combat_npc.h"
#include "combat_player.h"
#include "combat_def.h"
#include "world_boss.h"
#include "mob_spawner.h"
#include "mob_manager.h"
#include "global_drop.h"
#include "unit_state.h"
#include "npc_lua_state.h"
#include "combat_lstate_man.h"
#include "obj_query.h"

#include "shared/net/packet/platform_define.h"


namespace combat
{

/***********************************CombatMan***********************************/
/***********************************CombatMan***********************************/
/***********************************CombatMan***********************************/
/***********************************CombatMan***********************************/

CombatMan::CombatMan()
	: tick_counter_(0)
{
}

CombatMan::~CombatMan()
{
}

bool CombatMan::Initialize(std::vector<int>& mapid_vec, std::string& combat_script_path, const std::vector<cls_prop_sync_rule>& list)
{
	//初始化战斗对象状态
	UnitState::InitStateTable();

	//初始化怪物数据
	s_mob_man.Initialize();

	//初始化魔偶数据
	s_golem_man.Initialize();

	//初始化怪物组数据
	s_mob_spawner.Initialize();

	//初始化全局掉落
	s_global_drop_man.Initialize(mapid_vec);

	//初始化和lua通信的栈
	s_npc_lstate_man.Initialize(combat_script_path);

	//初始化场景AI的lua_state管理器
	s_combat_lstate_man.Initialize(combat_script_path);

	//初始化组队奖励配置
	CombatAwardConfig::InitTeamAwardConfig();

	//初始化角色属性规则
	PlayerPropRuler::InitClsPropSyncRule(list);

    //初始化战场状态机
    CombatState::Initialize();

	//初始化对象池
#if defined(SERVER_SIDE)
	pve_combat_man_.Init(PVE_COMBAT_SIZE);
	pvp_combat_man_.Init(PVP_COMBAT_SIZE);
	combat_player_man_.Init(COMBAT_PLAYER_SIZE);
	combat_npc_man_.Init(COMBAT_NPC_SIZE);
	world_boss_man_.Init(WORLD_BOSS_SIZE);
#elif defined(CLIENT_SIDE)
	pve_combat_man_.Init(1);
	pvp_combat_man_.Init(1);
	combat_player_man_.Init(1);
	combat_npc_man_.Init(4);
	world_boss_man_.Init(1);
#endif

	return true;
}

void CombatMan::Release()
{
	pve_combat_man_.Release();
	pvp_combat_man_.Release();
	combat_player_man_.Release();
	combat_npc_man_.Release();
	world_boss_man_.Release();
	s_combat_lstate_man.Release();
}

void CombatMan::HeartBeat()
{
	pve_combat_man_.HeartBeat();
	pvp_combat_man_.HeartBeat();
	combat_player_man_.HeartBeat();
	combat_npc_man_.HeartBeat();
	world_boss_man_.HeartBeat();

	// 分发消息
	msg_queue_.HeartBeat();

	// test
	if (++ tick_counter_ >= 3000)
	{
		tick_counter_ = 0;
		Trace();
	}
}

#ifndef DEFINE_OBJ_MAN_OPERATION_FUNC
#define DEFINE_OBJ_MAN_COMMON_OP_FUNC(class_name, NAME, name) \
        class_name* CombatMan::Query##NAME(ObjectID id) \
		{ \
			if (id > 0) \
			{ \
				size_t index = ID2INDEX(id); \
				return name##_man_.GetByIndex(index); \
			} \
			else \
			{ \
				return NULL; \
			} \
		} \
		void CombatMan::Insert##NAME##ToMan(class_name* obj) \
		{ \
			name##_man_.Insert(obj); \
		} \
		void CombatMan::Remove##NAME##FromMan(class_name* obj) \
		{ \
			name##_man_.Remove(obj); \
		} \
		size_t CombatMan::GetObjIndex(const class_name* obj) const \
		{ \
			return name##_man_.GetIndex(obj); \
		}

#define DEFINE_OBJ_MAN_OPERATION_FUNC(class_name, NAME, name) \
		class_name* CombatMan::Alloc##NAME() \
		{ \
			class_name* obj = name##_man_.Alloc(); \
			if (NULL == obj) \
			{ \
				__PRINTF("战斗系统对象池耗尽,对象类型(%d)", name##_man_.GetByIndex(0)->GetType()); \
			} \
			return obj; \
		} \
		void CombatMan::Free##NAME(class_name* obj) \
		{ \
			name##_man_.Free(obj); \
		} \
        DEFINE_OBJ_MAN_COMMON_OP_FUNC(class_name, NAME, name)
		
DEFINE_OBJ_MAN_OPERATION_FUNC(CombatPVE, PVECombat, pve_combat);
DEFINE_OBJ_MAN_OPERATION_FUNC(CombatPVP, PVPCombat, pvp_combat);
DEFINE_OBJ_MAN_OPERATION_FUNC(CombatNpc, NPC, combat_npc);
DEFINE_OBJ_MAN_COMMON_OP_FUNC(CombatPlayer, Player, combat_player);
DEFINE_OBJ_MAN_COMMON_OP_FUNC(WorldBoss, WorldBoss, world_boss);

#undef DEFINE_OBJ_MAN_COMMON_OP_FUNC
#undef DEFINE_OBJ_MAN_OPERATION_FUNC
#endif

CombatPlayer* CombatMan::AllocPlayer(RoleID role_id)
{
    CombatPlayer* pplayer = combat_player_man_.Alloc();
    if (NULL == pplayer)
    {
		__PRINTF("战斗系统对象池耗尽,对象类型(%d)", combat_player_man_.GetByIndex(0)->GetType());
        return NULL;
    }

    size_t index = combat_player_man_.GetIndex(pplayer);
    if (!ObjQuery::MapPlayer(role_id, index))
    {
        combat_player_man_.Free(pplayer);
        __PRINTF("MapPlayer() error! player already in combat! roleid:%ld", role_id);
        return NULL;
    }
    return pplayer;
}

void CombatMan::FreePlayer(CombatPlayer* pplayer)
{
    ObjQuery::UnmapPlayer(pplayer->GetRoleID());
    //ASSERT(n == 1);
    combat_player_man_.Free(pplayer);
}

WorldBoss* CombatMan::AllocWorldBoss(ObjectID obj_id)
{
    WorldBoss* pboss = world_boss_man_.Alloc();
    if (NULL == pboss)
    {
		__PRINTF("战斗系统对象池耗尽,对象类型(%d)", world_boss_man_.GetByIndex(0)->GetType());
        return NULL;
    }

    size_t index = world_boss_man_.GetIndex(pboss);
    if (!ObjQuery::MapWorldBoss(obj_id, index))
    {
        world_boss_man_.Free(pboss);
        __PRINTF("MapWorldBoss() error! worldboss already in combat! obj_id:%d", obj_id);
        return NULL;
    }
    return pboss;
}

void CombatMan::FreeWorldBoss(WorldBoss* pboss)
{
    int n = ObjQuery::UnmapWorldBoss(pboss->GetWorldBossObjectID());
    ASSERT(n == 1);
    world_boss_man_.Free(pboss);
}

CombatUnit* CombatMan::QueryCombatUnit(ObjectID unit_id)
{
    Object* object = QueryObject(unit_id);
    if (!object)
    {
        return NULL;
    }
    return dynamic_cast<CombatUnit*>(object);
}

CombatUnit* CombatMan::QueryCombatUnit(const XID& xid)
{
    Object* object = QueryObject(xid.id);
    if (!object)
    {
        return NULL;
    }
    return dynamic_cast<CombatUnit*>(object);
}

Combat* CombatMan::QueryCombat(ObjectID combat_id)
{
    Object* object = QueryObject(combat_id);
    if (!object)
    {
        return NULL;
    }
    return dynamic_cast<Combat*>(object);
}

Combat* CombatMan::QueryCombat(const XID& xid)
{
    Object* object = QueryObject(xid.id);
    if (!object)
    {
        return NULL;
    }
    return dynamic_cast<Combat*>(object);
}

WorldBoss* CombatMan::QueryBoss(ObjectID unit_id)
{
	return QueryWorldBoss(unit_id);
}

WorldBoss* CombatMan::QueryBoss(const XID& xid)
{
	return QueryWorldBoss(xid.id);
}

Object* CombatMan::QueryObject(ObjectID object_id)
{
	int type = ID2Type(object_id);
	switch (type)
	{
		case OBJ_TYPE_PLAYER:
			return QueryPlayer(object_id);
		case OBJ_TYPE_NPC:
			return QueryNPC(object_id);
		case OBJ_TYPE_COMBAT_PVE:
			return QueryPVECombat(object_id);
		case OBJ_TYPE_COMBAT_PVP:
			return QueryPVPCombat(object_id);
		case OBJ_TYPE_WORLD_BOSS:
			return QueryWorldBoss(object_id);
		default:
			break;
	};
	return NULL;
}

CombatPlayer* CombatMan::FindPlayer(int64_t roleid)
{
    size_t index = std::numeric_limits<size_t>::max();
    if (!ObjQuery::GetPlayerIndex(roleid, index))
        return NULL;

    return combat_player_man_.GetByIndex(index);
}

WorldBoss* CombatMan::FindWorldBoss(ObjectID bossid)
{
    size_t index = std::numeric_limits<size_t>::max();
    if (!ObjQuery::GetWorldBossIndex(bossid, index))
        return NULL;

    return world_boss_man_.GetByIndex(index);
}

void CombatMan::SendMSG(const MSG& msg, size_t latency)
{
	msg_queue_.AddMsg(msg, latency);
}

void CombatMan::SendMSG(const ObjectVec& list, const MSG& msg, size_t latency)
{
	msg_queue_.AddMultiMsg(list, msg, latency);
}

void CombatMan::SendMSG(const XID* first, const XID* last, const MSG& msg, size_t latency)
{
	msg_queue_.AddMultiMsg(first, last, msg, latency);
}

void CombatMan::DispatchMSG(Object* object, const MSG& msg)
{
	shared::MutexLockGuard keeper(object->GetMutex());
	if (object->GetID() != msg.target.id || !object->IsActived())
	{
		assert(false);
		return;
	}
	object->MessageHandler(msg);
}

void CombatMan::DispatchMSG(const MSG& msg)
{
	Object* obj = s_pCombatMan->QueryObject(msg.target.id);
	if (!obj || !obj->IsActived())
	{
		//assert(false);
		return;
	}

	DispatchMSG(obj, msg);
}

void CombatMan::Trace() const
{
#ifdef _COMBAT_DUMP
	__PRINTF("----------------------对象池统计表----------------------");
	__PRINTF("CombatPVE   : Total(%3ld)\tAlloced(%3ld)\tElementSize(%3ld)", pve_combat_man_.TotalCount(), pve_combat_man_.ObjCount(), pve_combat_man_.ElementSize());
	__PRINTF("CombatPVP   : Total(%3ld)\tAlloced(%3ld)\tElementSize(%3ld)", pvp_combat_man_.TotalCount(), pvp_combat_man_.ObjCount(), pvp_combat_man_.ElementSize());
	__PRINTF("CombatPlayer: Total(%3ld)\tAlloced(%3ld)\tElementSize(%3ld)", combat_player_man_.TotalCount(), combat_player_man_.ObjCount(), combat_player_man_.ElementSize());
	__PRINTF("CombatNpc   : Total(%3ld)\tAlloced(%3ld)\tElementSize(%3ld)", combat_npc_man_.TotalCount(), combat_npc_man_.ObjCount(), combat_npc_man_.ElementSize());
	__PRINTF("WorldBoss   : Total(%3ld)\tAlloced(%3ld)\tElementSize(%3ld)", world_boss_man_.TotalCount(), world_boss_man_.ObjCount(), world_boss_man_.ElementSize());

	s_combat_lstate_man.Trace();
#endif
}

};// namespace combat
