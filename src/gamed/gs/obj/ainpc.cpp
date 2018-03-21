#include "ainpc.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/types.h"
#include "gs/scene/world.h"

#include "npc.h"
#include "npc_sender.h"


namespace gamed {

using namespace npcdef;

namespace 
{
	// 转为百分数
	uint16_t speed_to_percentage(float speed)
	{
		return static_cast<uint16_t>(speed*100.f + 0.5f);
	}

} // Anonymous


///
/// AINpcObject
///
AINpcObject::AINpcObject(Npc& npc)
	: npc_(npc),
	  paggro_(NULL),
	  min_chase_range_(1000.f),
	  max_chase_range_(0.f),
	  is_going_home_(false)
{
}

AINpcObject::~AINpcObject()
{
}

void AINpcObject::SetAggroPolicy(AggroPolicy* aggro)
{
	paggro_ = aggro;
}

void AINpcObject::SetChaseRange(float min_range, float max_range)
{
	ASSERT(max_range > min_range);

	min_chase_range_ = min_range;
	max_chase_range_ = max_range;
}

WorldObject* AINpcObject::GetAIOwnerPtr()
{
	return static_cast<WorldObject*>(&npc_);
}

bool AINpcObject::CanRest()
{
	return npc_.CanRest();
}

void AINpcObject::AddSession(ActiveSession* psession)
{
	npc_.AddSessionAndStart(psession);
}

void AINpcObject::ClearSessions()
{
	npc_.ClearSessions();
}

bool AINpcObject::HasSession() const
{
	return npc_.HasSession();
}

void AINpcObject::GetPatrolPos(A2DVECTOR& pos) const
{
	npc_.GetBirthPlace(pos);
}

void AINpcObject::GetSelfPos(A2DVECTOR& pos) const
{
	pos = npc_.pos();
}

void AINpcObject::ReturnHome(const A2DVECTOR& target)
{
	uint16_t speed_to_client = speed_to_percentage(npc_.GetChaseSpeed());
	npc_.sender()->Move(target, 
				        kNpcPatrolUseMsecTime,
				        speed_to_client,
						movement::MOVE_MODE_RETURN);
	
	A2DVECTOR offset = target;
	offset -= npc_.pos();
	npc_.StepMove(offset);
}

void AINpcObject::SetGoingHome(AIObject::GoHameType go_type)
{
	if (go_type == AIObject::GHT_START_GOING)
	{
		// 不再主动搜寻目标
		SetAggroWatch(false);
	}
	else
	{
		if (!CanAggroWatch())
		{
			// 恢复搜索功能
			SetAggroWatch(true);
		}
	}

	is_going_home_ = (go_type == AIObject::GHT_REACH_GOAL) ? false : true;
}

bool AINpcObject::IsGoingHome() const
{
	return is_going_home_;
}

void AINpcObject::ReachDestination(const A2DVECTOR& pos)
{
    npc_.ReachDestination(pos);
}

size_t AINpcObject::GetAggroCount() const
{
	return paggro_->Size();
}

bool AINpcObject::GetFirstAggro(XID& id) const
{
	return paggro_->GetFirst(id);
}

bool AINpcObject::CanAggroWatch() const
{
	return paggro_->CanWatch();
}

void AINpcObject::SetAggroWatch(bool bRst)
{
	paggro_->SetAggroWatch(bRst);
}

void AINpcObject::HateTarget(const XID& target)
{
	npc_.SendMsg(GS_MSG_HATE_YOU, target);
}

bool AINpcObject::QueryTarget(const XID& xid, target_info& info)
{
	if (xid.IsPlayer())
	{
		world::player_base_info playerinfo;
		if (npc_.world_plane()->QueryPlayer(xid.id, playerinfo))
		{
			info.pos = playerinfo.pos;
			return true;
		}
	}
	else if (xid.IsNpc())
	{
		world::worldobj_base_info obj_info;
		if (npc_.world_plane()->QueryObject(xid, obj_info))
		{
			info.pos = obj_info.pos;
			return true;
		}
	}
	else
	{
		ASSERT(false);
		return false;
	}

	return false;
}

void AINpcObject::RemoveAggroEntry(const XID& xid)
{
	paggro_->Remove(xid);
}

void AINpcObject::ClearAggro()
{
	paggro_->Clear();
}

void AINpcObject::TriggerCombat(const XID& target)
{
	npc_.TriggerCombat(target);
}

void AINpcObject::GetChaseRange(float& min_range, float& max_range) const
{
	min_range = min_chase_range_;
	max_range = max_chase_range_;
}


///
/// NpcAI
///
NpcAI::NpcAI(Npc& npc)
	: obj_(npc),
	  core_(&obj_),
	  aggro_(&obj_)
{
}

NpcAI::~NpcAI()
{
}

bool NpcAI::Init(const AggroParam& aggro_param)
{
	if (!core_.Init())
	{
		LOG_ERROR << "npc aipolicy init error!";
		return false;
	}

	if (!aggro_.Init(aggro_param))
	{
		LOG_ERROR << "npc ai aggro init error!";
		return false;
	}

	// after aggro_.Init()
	obj_.SetAggroPolicy(&aggro_);
	return true;
}

bool NpcAI::ChangeAggroPolicy(const AggroParam& aggro_param)
{
	aggro_.ChangeAggroParam(aggro_param);
	return true;
}

bool NpcAI::AggroWatch(const XID& target, const A2DVECTOR& pos, int faction)
{
	if (aggro_.AggroWatch(target, pos, faction))
	{
		// 第一次产出仇恨，仇恨列表里第一位才调用OnAggro
		core_.OnAggro();
		return true;
	}
	return false;
}

bool NpcAI::ChangeWatch(const XID& target, const A2DVECTOR& pos, int faction)
{
	if (aggro_.ChangeWatch(target, pos, faction))
	{
		// 在追击过程中，改变仇恨目标（可能上次的仇恨对象已经进入战斗）
		return true;
	}
	return false;
}

bool NpcAI::CanAggroWatch() const
{
	if (!aggro_.CanWatch() || !aggro_.IsEmpty())
		return false;

	return true;
}

void NpcAI::SetChaseRange(float min_range, float max_range)
{
	obj_.SetChaseRange(min_range, max_range);
}

XID NpcAI::GetFirstAggro() const
{
	XID target(-1, -1);
	obj_.GetFirstAggro(target);
	return target;
}

void NpcAI::ClearAggro()
{
	obj_.ClearAggro();
}

void NpcAI::UnderAttack(const XID& attacker)
{
	if (core_.UnderAttack(attacker))
	{
		aggro_.AddToFirst(attacker, 1);
	}
}

void NpcAI::RelieveAttack()
{
	core_.RelieveAttack();
}

void NpcAI::SetAggressiveMode(bool bRst)
{
	if (bRst)
	{
		core_.SetPrimaryStrategy(AIPolicy::STRATEGY_AGGRESSIVE);
	}
	else
	{
		core_.SetPrimaryStrategy(AIPolicy::STRATEGY_DEFENSIVE);
	}
}

void NpcAI::Heartbeat()
{
	core_.Heartbeat();
}

void NpcAI::GoHome()
{
	core_.GoHome();
}

void NpcAI::ResetPolicy()
{
    core_.ResetPolicy();
}

void NpcAI::SomeoneGreeting(const XID& someone)
{
	core_.OnGreeting(someone);
}

} // namespace gamed
