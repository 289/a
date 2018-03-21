#include "aiaggro.h"

#include "gs/global/game_def.h"
#include "gs/global/math_types.h"

#include "faction.h"
#include "aipolicy.h"
#include "object.h"


namespace gamed {

///
/// AggroParam
///
AggroParam::AggroParam()
	: aggro_range(kNpcDefaultAggroRange),
	  aggro_time(kNpcDefaultAggroTime),
	  faction(FACTION_NONE),
	  enemy_faction(FACTION_NONE)
{
}


///
/// AggroPolicy
///
AggroPolicy::AggroPolicy(AIObject* obj)
	: self_(obj),
	  timeout_timer_(0),
	  aggro_watch_(true)
{
}

AggroPolicy::~AggroPolicy()
{
}

bool AggroPolicy::Init(const AggroParam& param)
{
	ChangeAggroParam(param);
	return true;
}

void AggroPolicy::ChangeAggroParam(const AggroParam& param)
{
	aggro_range_     = param.aggro_range;
	aggro_time_      = param.aggro_time;
	faction_         = param.faction;
	enemy_faction_   = param.enemy_faction;
	aggro_max_range_ = param.aggro_range * 2;
}

bool AggroPolicy::GetFirst(XID& target)
{
	return aggro_list_.GetFirst(target);
}

int AggroPolicy::AddAggro(const XID& id, int rage)
{
	return aggro_list_.AddRage(id, rage);
}

void AggroPolicy::Heartbeat()
{
	if (timeout_timer_ && --timeout_timer_ <= 0)
	{
		// 超时
		aggro_list_.RemoveFirst();
		// 重新选目标
		XID target;
		if (aggro_list_.GetFirst(target))
		{
			timeout_timer_ = aggro_time_;
		}
		else
		{
			timeout_timer_ = 0;
			Clear();
		}
	}
}

bool AggroPolicy::AggroWatch(const XID& target, const A2DVECTOR& pos, int faction)
{
	if (!aggro_watch_)
		return false;

	// IsEmpty()的判断已经做了优化，提前至monster_imp
	if (aggro_list_.IsEmpty())
	{
		A2DVECTOR self_pos = self_->GetAIOwnerPtr()->pos();
		if (self_pos.squared_distance(pos) > aggro_range_*aggro_range_)
			return false;

		if (faction & enemy_faction_)
		{
			AddAggro(target, 1);
			timeout_timer_ = aggro_time_;
			return true;
		}
	}

	return false;
}

bool AggroPolicy::ChangeWatch(const XID& target, const A2DVECTOR& pos, int faction)
{
	A2DVECTOR self_pos = self_->GetAIOwnerPtr()->pos();
	if (self_pos.squared_distance(pos) > aggro_range_*aggro_range_)
		return false;

	if (faction & enemy_faction_)
	{
		AddToFirst(target, 1);
		timeout_timer_ = aggro_time_;
		return true;
	}

	return false;
}

void AggroPolicy::Remove(const XID& id)
{
	if (aggro_list_.Remove(id) == 0)
	{
		timeout_timer_ = aggro_time_;
	}
}

void AggroPolicy::Clear()
{
	timeout_timer_ = 0;
	aggro_list_.Clear();
}

int AggroPolicy::AddToFirst(const XID& id, int addon_rage)
{
	return aggro_list_.AddToFirst(id, addon_rage);
}

} // namespace gamed
