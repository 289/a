#include "monster_imp.h"

#include "game_module/pathfinder/include/path_finder.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/math_types.h"
#include "gs/global/game_def.h"
#include "gs/global/game_util.h"
#include "gs/scene/world.h"

#include "npc_def.h"
#include "aiaggro.h"
#include "faction.h"
#include "ainpc.h"
#include "npcsession.h"


namespace gamed {

using namespace mapDataSvr;
using namespace npcdef;


///
/// MonsterImp
///
MonsterImp::MonsterImp(Npc& npc)
	: NpcImp(npc),
	  state_(MON_STATE_PEACE),
	  waiting_combat_timer_(0),
	  is_retain_model_(false),
	  monster_type_(MSG_MT_NORMAL),
      path_move_speed_(-1),
      can_combat_(true)
{
}

MonsterImp::~MonsterImp()
{
	state_                = MON_STATE_PEACE;
	waiting_combat_timer_ = 0;
	is_retain_model_      = false;
	monster_type_         = MSG_MT_NORMAL;
}

bool MonsterImp::OnInit()
{
	const BaseMapData* pmapdata = s_pMapData->QueryBaseMapDataTempl(npc_.elem_id());
	if (pmapdata == NULL)
	{
		LOG_ERROR << "MonsterImp::OnInit() failure! reason:template not found, "
			<< "or tamplate type invalid! templ_id" << npc_.templ_id()
			<< " elem_id" << npc_.elem_id();
		return false;
	}

	int type = pmapdata->GetType();
	switch (type)
	{
		case MAPDATA_TYPE_SPOT_MONSTER:
			{
				const SpotMonster* pmonster = dynamic_cast<const SpotMonster*>(pmapdata);
				if (pmonster == NULL) 
					return false;

				FillElemInfo(pmonster);
			}
			break;

		case MAPDATA_TYPE_AREA_MONSTER:
			{
				const AreaMonster* pmonster = dynamic_cast<const AreaMonster*>(pmapdata);
				if (pmonster == NULL)
					return false;

				FillElemInfo(pmonster);
			}
			break;

		default:
			{
				LOG_ERROR << "MonsterImp::OnInit() template type invalid! type:" << type 
					<< " elem_id" << npc_.elem_id(); 
			}
			return false;
	}
	
	// always the last!
	if (!InitRuntimeData())
		return false;

	return true;
}

void MonsterImp::FillElemInfo(const SpotMonster* pmonster)
{
	elem_info_.birth_place        = npc_.pos();
	elem_info_.birth_dir          = npc_.dir();
	elem_info_.monster_group_id   = pmonster->monster_group_id;
	elem_info_.battle_scene_id    = pmonster->battle_scene_id;
	elem_info_.ap_mode            = pmonster->ap_mode;
	elem_info_.is_wandered        = pmonster->is_wandered;
	elem_info_.combat_rule        = pmonster->combat_rule;
	elem_info_.refresh_interval   = pmonster->refresh_interval;
	elem_info_.patrol_path_id     = pmonster->patrol_path_id;
	elem_info_.patrol_mode        = pmonster->patrol_mode;
	elem_info_.aggro_view_radius  = pmonster->aggro_view_radius;
	elem_info_.aggro_time         = pmonster->aggro_time;
	elem_info_.max_chase_distance = pmonster->max_chase_distance;
	elem_info_.chase_speed        = pmonster->chase_speed;

	aggressive_mode_ = (elem_info_.ap_mode == mapDataSvr::ACTIVE) ? true : false;
	is_retain_model_ = (elem_info_.combat_rule == mapDataSvr::NO_MODEL) ? false : true;
	monster_type_    = (elem_info_.combat_rule == mapDataSvr::MODEL_WORLD_BOSS) ? MSG_MT_WORLD_BOSS : MSG_MT_NORMAL;
    can_combat_      = (elem_info_.combat_rule == mapDataSvr::NEVER_COMBAT) ? false : true;
}

void MonsterImp::FillElemInfo(const AreaMonster* pmonster)
{
	elem_info_.birth_place        = npc_.pos();
	elem_info_.birth_dir          = npc_.dir();
	elem_info_.monster_group_id   = pmonster->monster_group_id;
	elem_info_.battle_scene_id    = pmonster->battle_scene_id;
	elem_info_.ap_mode            = pmonster->ap_mode;
	elem_info_.is_wandered        = pmonster->is_wandered;
	elem_info_.combat_rule        = pmonster->combat_rule;
	elem_info_.refresh_interval   = pmonster->refresh_interval;
	elem_info_.patrol_path_id     = 0;
	elem_info_.patrol_mode        = 0;
	elem_info_.aggro_view_radius  = pmonster->aggro_view_radius;
	elem_info_.aggro_time         = pmonster->aggro_time;
	elem_info_.max_chase_distance = pmonster->max_chase_distance;
	elem_info_.chase_speed        = pmonster->chase_speed;

	aggressive_mode_ = (elem_info_.ap_mode == mapDataSvr::ACTIVE) ? true : false;
	is_retain_model_ = (elem_info_.combat_rule == mapDataSvr::NO_MODEL) ? false : true;
	monster_type_    = (elem_info_.combat_rule == mapDataSvr::MODEL_WORLD_BOSS) ? MSG_MT_WORLD_BOSS : MSG_MT_NORMAL;
    can_combat_      = (elem_info_.combat_rule == mapDataSvr::NEVER_COMBAT) ? false : true;
}

bool MonsterImp::InitRuntimeData()
{
	// 初始化仇恨策略
	AggroParam param;
	param.aggro_range    = elem_info_.aggro_view_radius;
	param.aggro_time     = elem_info_.aggro_time;
	param.faction        = FACTION_MONSTER;
	param.enemy_faction  = GetNormalMonsterEnemyMask();
	if (!npc_.aicore()->ChangeAggroPolicy(param))
	{
		return false;
	}

	// 设置追击距离
	float max_range = elem_info_.max_chase_distance;
	ASSERT(kAggroNpcMaxCombatDis < max_range);
	npc_.aicore()->SetChaseRange(kAggroNpcMaxCombatDis, max_range);

	// 设置主动被动
	npc_.aicore()->SetAggressiveMode(is_aggressive_mode());

	return true;
}

bool MonsterImp::OnCanStroll()
{
	return elem_info_.is_wandered;
}

int MonsterImp::GetRebornInterval()
{
	return elem_info_.refresh_interval;
}

void MonsterImp::TriggerCombat(const XID& target)
{
	ASSERT(target.IsPlayer());

	// check status
	if (state_ != MON_STATE_PEACE)
		return;
    
	// 查找该玩家状态
	world::player_base_info player_info;
	if (npc_.world_plane()->QueryPlayer(target.id, player_info))
	{
		if (!player_info.can_combat)
		{
			// 该玩家已经进入战斗
			npc_.aicore()->ClearAggro();
			SearchPlayer(BIND_MEM_CB(&NpcAI::ChangeWatch, npc_.aicore()));
			return;
		}

		msg_obj_trigger_combat param;
		param.monster_type     = static_cast<int8_t>(monster_type_);
		param.monster_group_id = elem_info_.monster_group_id;
		param.battle_scene_id  = elem_info_.battle_scene_id;
		param.require_combatend_notify = is_retain_model_;
		npc_.SendMsg(GS_MSG_OBJ_TRIGGER_COMBAT, target, &param, sizeof(param));

		// 设置状态
		state_                = MON_STATE_WAITING_COMBAT;
		waiting_combat_timer_ = kWaitingCombatTimeoutSecs;
	}
}

float MonsterImp::GetChaseSpeed() const
{
    if (has_path_move())
        return path_move_speed_;

	return elem_info_.chase_speed;
}

void MonsterImp::OnHeartbeat()
{
	// state switch func
	DriveMachine();
}

int MonsterImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_PLAYER_TRIGGER_COMBAT:
			{
				// 战斗不管能否触发都要做出回应
				RoleID playerid = msg.source.id;
				HandlePlayerTriggerCombat(playerid);
			}
			break;

		case GS_MSG_OBJ_TRIGGER_COMBAT_RE:
			{
				if ( state_ == MON_STATE_WAITING_COMBAT &&
					 npc_.aicore()->GetFirstAggro() == msg.source)
				{
					CHECK_CONTENT_PARAM(msg, msg_obj_trigger_combat_re);
					msg_obj_trigger_combat_re& param = *(msg_obj_trigger_combat_re*)msg.content;
					HandleCombatStart(param.is_success, msg.source, true);
				}
			}
			break;

		case GS_MSG_COMBAT_PVE_END:
			{
				CHECK_CONTENT_PARAM(msg, msg_combat_end);
				msg_combat_end* param = (msg_combat_end*)msg.content;
				HandleCombatEnd(*param);
			}
			break;

        case GS_MSG_P_TRIGGER_COMBAT_SUCCESS:
            {
                ASSERT(msg.source.IsPlayer());
                HandlePTriggerCombatSuccess();
            }
            break;

        default:
            ASSERT(false);
            return -1;
	}

	return 0;
}

bool MonsterImp::OnMapTeleport()
{
    if (state_ != MON_STATE_PEACE)
    {
        return false;
    }

    // 先清空session
    npc_.AddSessionAndStart(new NpcEmptySession(&npc_));

    // ai模块重置
    npc_.aicore()->ResetPolicy();

    return true;
}

bool MonsterImp::OnMonsterMove(A2DVECTOR pos, float speed)
{
    if (state_ != MON_STATE_PEACE)
    {
        return false;
    }

    // 设置速度
    path_move_speed_ = (speed < 0.f) ? 0.05 : speed;

    // 先清空session
    npc_.AddSessionAndStart(new NpcEmptySession(&npc_));

    // ai模块重置
    npc_.aicore()->ResetPolicy();

    // go home
    npc_.aicore()->GoHome();

    return true;
}

void MonsterImp::OnSetMonsterSpeed(float speed)
{
    path_move_speed_ = (speed < 0.f) ? 0.05 : speed;
}

void MonsterImp::OnReachDestination(const A2DVECTOR& pos)
{
    if (has_path_move())
    {
        plane_msg_reach_destination param;
        param.elem_id = npc_.elem_id();
        param.pos_x   = pos.x;
        param.pos_y   = pos.y;
        npc_.SendPlaneMsg(GS_PLANE_MSG_REACH_DESTINATION, &param, sizeof(param));

        path_move_speed_ = -1;
    }
}

bool MonsterImp::OnPauseAICore()
{
    // 战斗及战斗等待状态时暂停AI模块
    if (!CheckStateToCombat())
        return true;

    // 和平状态不需要暂停
    return false;
}

bool MonsterImp::OnCanCombat() const
{
    return can_combat_;
}

bool MonsterImp::CheckStateToCombat()
{
	// 是否在等待战斗，或者已经进入战斗
	if (state_ == MON_STATE_WAITING_COMBAT || state_ == MON_STATE_COMBAT)
	{
		return false;
	}

	return true;
}

bool MonsterImp::CheckCombatCondition(RoleID playerid)
{
	// 检查自身状态,是否可战斗,Boss在战斗状态下还可以触发战斗
	if (!CheckStateToCombat())
	{
		return false;
	}
	
	// 查找该玩家
	world::player_base_info player_info;
	if (!npc_.world_plane()->QueryPlayer(playerid, player_info))
	{
		return false;
	}

	// 检查距离条件
	if (maths::squared_distance(npc_.pos(), player_info.pos) > kMaxCombatDistanceSquare)
	{
		return false;
	}

	return true;
}

void MonsterImp::ReplyCombatTrigger(RoleID playerid, bool is_trigger_success)
{
	msg_player_trigger_combat_re param;
	param.is_success = is_trigger_success;
	param.require_combatend_notify = is_retain_model_;
	param.monster_type = static_cast<int8_t>(monster_type_);
	if (is_trigger_success)
	{
		param.battle_scene_id = elem_info_.battle_scene_id;
		param.monster_group_id = elem_info_.monster_group_id;
	}
	else
	{
		param.battle_scene_id = 0;
		param.monster_group_id = 0;
	}

	// 一定要发送Replay给战斗的触发者
	XID target;
	target.id   = playerid;
	target.type = XID::TYPE_PLAYER;
	npc_.SendMsg(GS_MSG_PLAYER_TRIGGER_COMBAT_RE, target, &param, sizeof(param));

	if (is_trigger_success)
	{
		HandleCombatStart(is_trigger_success, target);
	}
}

void MonsterImp::HandlePlayerTriggerCombat(RoleID playerid)
{
    // 重生状态不能触发战斗
    if (CheckRebornState())
    {
		ReplyCombatTrigger(playerid, false);
        return;
    }

	if (CheckCombatCondition(playerid))
	{
		ReplyCombatTrigger(playerid, true);
	}
	else
	{
		ReplyCombatTrigger(playerid, false);
	}
}

void MonsterImp::HandleCombatStart(bool is_success, XID attacker, bool is_player_reply)
{
    // 重生状态就不处理消息
    if (CheckRebornState())
        return;

    // 已经在战斗中
    if (get_state() == MON_STATE_COMBAT)
        return;

	if (is_success)
	{
		ASSERT(attacker.IsPlayer() || attacker.IsNpc());
		// 先清空session
		npc_.AddSessionAndStart(new NpcEmptySession(&npc_));

		// 受击处理
		state_ = MON_STATE_COMBAT;
		npc_.aicore()->UnderAttack(attacker);

		// 设置zombie状态
		if (is_retain_model_)
		{
            if (is_player_reply)
            {
                // 是怪发起的，直接置为僵尸状态
			    npc_.SetZombieState(kMonsterModelMaxRetainSecs);
            }
            else
            {
                // 是玩家发起的，还要等待玩家那边战斗创建成功
                state_                = MON_STATE_WAITING_COMBAT;
                waiting_combat_timer_ = kWaitingCombatTimeoutSecs;
            }
		}
		else
		{
			npc_.RebornAtOnce();
		}
	}
	else 
	{
		npc_.aicore()->ClearAggro();
		state_ = MON_STATE_PEACE;

		// 主动怪主动追击时，如果玩家不满足条件自动搜索边上玩家
		if (is_player_reply && is_aggressive_mode())
		{
			SearchPlayer(BIND_MEM_CB(&NpcAI::ChangeWatch, npc_.aicore()));
		}
	}
}

void MonsterImp::HandleCombatEnd(const msg_combat_end& msg_param)
{
    // 重生状态就不处理消息
    if (CheckRebornState())
        return;

    // 已经不再战斗中
    if (get_state() != MON_STATE_COMBAT)
        return;

	if (msg_param.combat_is_win())
	{
		// 怪物胜利
        npc_.aicore()->ClearAggro();
        npc_.aicore()->RelieveAttack();
        state_ = MON_STATE_PEACE;
        npc_.SetNormalState();

        // 尝试回出生点
		TryGoHome();
	}
	else
	{
		// 怪输了就开始刷新
		npc_.RebornAtOnce();
	}
}

void MonsterImp::HandlePTriggerCombatSuccess()
{
    state_ = MON_STATE_COMBAT;
    npc_.SetZombieState(kMonsterModelMaxRetainSecs);
}

void MonsterImp::AggressiveSearch()
{
	// 提前判断，减少QueryPlayer次数
	if (!npc_.aicore()->CanAggroWatch())
		return;

	SearchPlayer(BIND_MEM_CB(&NpcAI::AggroWatch, npc_.aicore()));
}

bool MonsterImp::SearchPlayer(const SearchCallback& callback)
{
	// 不是主动怪不能搜索玩家
	if (!is_aggressive_mode())
		return false;

	const Npc::InViewPlayersSet& playersSet  = npc_.GetPlayersInView();
	Npc::InViewPlayersSet::const_iterator it = playersSet.begin();
	for (; it != playersSet.end(); ++it)
	{
		RoleID roleid = *it;
		world::player_base_info info;
		if (npc_.world_plane()->QueryPlayer(roleid, info))
		{
			// 玩家正在战斗则不选为目标
			if (!info.can_combat)
				continue;
			// 免疫主动怪的玩家不能被选为目标
			if (info.immune_active_monster())
				continue;
			// 通过图上直线可达才主动出击
			if (!pathfinder::LineTo(npc_.world_id(), A2D_TO_PF(npc_.pos()), A2D_TO_PF(info.pos)))
				continue;
			// watch 成功则break
			if (callback(info.xid, info.pos, info.faction))
				return true;
		}
	}

	return false;
}

void MonsterImp::DriveMachine()
{
	switch (state_)
	{
		case MON_STATE_PEACE:
			{
				// 主动怪
				if (is_aggressive_mode())
				{
					AggressiveSearch();
				}
			}
			break;

		case MON_STATE_WAITING_COMBAT:
			{
				if (--waiting_combat_timer_ < 0)
				{
					HandleCombatStart(false, XID(-1, -1));
					waiting_combat_timer_ = 0xFFFF;
				}
			}
			break;

		case MON_STATE_COMBAT:
			break;

		default:
			ASSERT(false);
	}
}

void MonsterImp::TryGoHome()
{
	// 可以移动的怪，主动回到出生点
	if (aggressive_mode_ || OnCanStroll())
	{
		npc_.aicore()->GoHome();
	}
}

bool MonsterImp::CheckRebornState() const
{
    if (npc_.state() == npcdef::STATE_REBORN)
    {
        return true;
    }
    return false;
}

void MonsterImp::SetBirthPlace(const A2DVECTOR& pos)
{
    elem_info_.birth_place = pos;
}

} // namespace gamed
