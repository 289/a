#include <assert.h>
#include <algorithm>
#include "combat_atb.h"
#include "combat_def.h"
#include "combat_unit.h"
#include "combat.h"

namespace combat
{

/**********************************CombatATB******************************/
/**********************************CombatATB******************************/
/**********************************CombatATB******************************/
/**********************************CombatATB******************************/
bool CombatATB::ATBFinder::operator()(const ATBSet::value_type& v) const
{
	return v.unit->GetID() == unit_id;
}

struct OBJFinder
{
	UnitID unit_id;
	OBJFinder(UnitID id): unit_id(id) {}
	bool operator() (const CombatUnit* obj) const
	{
		return obj->GetID() == unit_id;
	}
};

void CombatATB::DeActivate()
{
    assert(active_);
    active_ = false;

	__PRINTF("----------------------ATBMan被暂停了.");
}

void CombatATB::Activate()
{
    assert(!active_);
    active_ = true;

	__PRINTF("----------------------ATBMan恢复正常了.");
}

void CombatATB::Init()
{
	//ChangeState(STATUS_OPEN_READY, COMBAT_START_READY_TIME / MSEC_PER_TICK);
}

void CombatATB::RegisterATB(CombatUnit* unit)
{
	if (!unit || IsATBExist(unit->GetID()))
	{
		return;
	}

	ATBEntry entry;
	entry.interval = unit->GetATBTime();
	entry.priority = unit->GetAttackPriority();
	entry.remain_time = unit->GetATBTime();
	entry.tick = entry.interval / MSEC_PER_TICK;
	entry.unit = unit;
	atb_set_.insert(entry);

	__PRINTF("RegisterATB: 注册ATB. unit_id = %d, interval = %dms.", unit->GetID(), entry.interval);
}

void CombatATB::RegisterInstantATB(CombatUnit* unit)
{
	if (!unit)
		return;

	if (IsATBExist(unit->GetID()) && unit != cur_obj_)
	{
		UnRegisterATB(unit->GetID());
	}

	//插入即时ATB集合
	instant_obj_vec_.push_back(unit);

	__PRINTF("RegisterATB: 注册即时ATB. unit_id = %d.", unit->GetID());
}

void CombatATB::ResetATB(CombatUnit* unit)
{
	if (!unit)
		return;

	if (IsATBExist(unit->GetID()))
	{
		UnRegisterATB(unit->GetID());
	}

	RegisterATB(unit);

	__PRINTF("RegisterATB: 重置ATB. unit_id = %d, interval = %dms.", unit->GetID(), unit->GetATBTime());
}

void CombatATB::UnRegisterATB(UnitID unit_id)
{
	if (!IsATBExist(unit_id))
	{
		__PRINTF("UnRegisterATB: 删除ATB失败!!! unit_id = %d.", unit_id);
		return;
	}

	ATBSet::iterator ait = std::find_if(atb_set_.begin(), atb_set_.end(), ATBFinder(unit_id));
	if (ait != atb_set_.end())
	{
		__PRINTF("UnRegisterATB: atb_set_ unit_id = %d.", unit_id);
		atb_set_.erase(ait);
        return;
	}

    OBJList::iterator rit = std::find_if(ready_obj_list_.begin(), ready_obj_list_.end(), OBJFinder(unit_id));
    if (rit != ready_obj_list_.end())
    {
        __PRINTF("UnRegisterATB: ready_obj_list_ unit_id = %d.", unit_id);
        ready_obj_list_.erase(rit);
        return;
    }

    OBJVec::iterator it = std::find_if(instant_obj_vec_.begin(), instant_obj_vec_.end(), OBJFinder(unit_id));
    if (it != instant_obj_vec_.end())
    {
        __PRINTF("UnRegisterATB: instant_obj_vec_ unit_id = %d.", unit_id);
        instant_obj_vec_.erase(it);
        return;
    }

    it = std::find_if(change_obj_vec_.begin(), change_obj_vec_.end(), OBJFinder(unit_id));
    if (it != change_obj_vec_.end())
    {
        __PRINTF("UnRegisterATB: change_obj_vec_ unit_id = %d.", unit_id);
        change_obj_vec_.erase(it);
        return;
    }
/*
	bool deleted = false;
	ATBSet::iterator it = std::find_if(atb_set_.begin(), atb_set_.end(), ATBFinder(unit_id));
	if (it != atb_set_.end())
	{
		__PRINTF("UnRegisterATB: atb_set_ unit_id = %d.", unit_id);
		atb_set_.erase(it);
		deleted = true;
	}

	if (!deleted)
	{
		OBJList::iterator it = std::find_if(ready_obj_list_.begin(), ready_obj_list_.end(), OBJFinder(unit_id));
		if (it != ready_obj_list_.end())
		{
		    __PRINTF("UnRegisterATB: ready_obj_list_ unit_id = %d.", unit_id);
			ready_obj_list_.erase(it);
			deleted = true;
		}
	}

	if (!deleted)
	{
		OBJVec::iterator it = std::find_if(instant_obj_vec_.begin(), instant_obj_vec_.end(), OBJFinder(unit_id));
		if (it != instant_obj_vec_.end())
		{
			instant_obj_vec_.erase(it);
			deleted = true;
		}
	}

	if (!deleted)
	{
		OBJVec::iterator it = std::find_if(change_obj_vec_.begin(), change_obj_vec_.end(), OBJFinder(unit_id));
		if (it != change_obj_vec_.end())
		{
		    __PRINTF("UnRegisterATB: change_obj_vec_ unit_id = %d.", unit_id);
			change_obj_vec_.erase(it);
			deleted = true;
		}
	}

	assert(deleted);
	__PRINTF("UnRegisterATB: 删除ATB. unit_id = %d.", unit_id);*/
}

void CombatATB::UnRegisterAllATB()
{
	std::vector<UnitID> unit_vec;

	ATBSet::iterator it_atb_set = atb_set_.begin();
	for (; it_atb_set != atb_set_.end(); ++ it_atb_set)
		unit_vec.push_back(it_atb_set->unit->GetID());

	for (size_t i = 0; i < change_obj_vec_.size(); ++ i)
		unit_vec.push_back(change_obj_vec_[i]->GetID());

	for (size_t i = 0; i < instant_obj_vec_.size(); ++ i)
		unit_vec.push_back(instant_obj_vec_[i]->GetID());

	OBJList::iterator it_ready_obj = ready_obj_list_.begin();
	for (; it_ready_obj != ready_obj_list_.end(); ++ it_ready_obj)
		unit_vec.push_back((*it_ready_obj)->GetID());

	if (unit_vec.empty())
		return;

	char buf[1024];
	memset(&buf, 0, sizeof(buf) / sizeof(char));
	sprintf((char*)(buf), "UnRegisterAllATB: ");
	for (size_t i = 0; i < unit_vec.size(); ++ i)
		sprintf((char*)(buf+strlen(buf)), "%d, ", unit_vec[i]);
	__PRINTF("%s", buf);

	atb_set_.clear();
	ready_obj_list_.clear();
	change_obj_vec_.clear();
	instant_obj_vec_.clear();
}

void CombatATB::OnATBChange(CombatUnit* unit)
{
    for (size_t i = 0; i < change_obj_vec_.size(); ++ i)
        if (change_obj_vec_[i] == unit)
            return;

	change_obj_vec_.push_back(unit);
}

void CombatATB::HeartBeat()
{
	if (!active_)
    {
        //ATB被暂停
        return;
    }

    ++total_tick_;
	DoChange();
	CollectReadyATB();
	CollectInstantATB();
	DoAttack();
}

void CombatATB::CollectReadyATB()
{
	///
	/// 把ATB时间到的战斗对象放到ready列表
	///

	ATBSet::iterator it = atb_set_.begin();
	while (it != atb_set_.end())
	{
		it->tick -= 1;
		it->remain_time -= MSEC_PER_TICK;
		if (it->tick <= 0)
		{
			//ATB时间到,玩家可以行动
			CombatUnit* unit = it->unit;
			OBJList::iterator it2 = std::find_if(ready_obj_list_.begin(), ready_obj_list_.end(), OBJFinder(unit->GetID()));
			assert(it2 == ready_obj_list_.end());
			ready_obj_list_.push_back(unit);
			atb_set_.erase(it ++);
		}
		else
		{
			++ it;
		}
	}
}

void CombatATB::CollectInstantATB()
{
	///
	/// 把即时ATB战斗对象放到ready列表
	///

	OBJVec::reverse_iterator it = instant_obj_vec_.rbegin();
	for (; it != instant_obj_vec_.rend(); ++ it)
	{
		ready_obj_list_.push_front(*it);
	}

	instant_obj_vec_.clear();
}

void CombatATB::DoChange()
{
	OBJVec::iterator it = change_obj_vec_.begin();
	OBJVec::iterator ie = change_obj_vec_.end();
	for (; it != ie; ++ it)
	{
		CombatUnit* unit = *it;
		UnitID unit_id = unit->GetID();
		ATBSet::const_iterator it_set = std::find_if(atb_set_.begin(), atb_set_.end(), ATBFinder(unit_id));
		if (it_set != atb_set_.end())
		{
			const ATBEntry& old_atb = *it_set;
			assert(unit == old_atb.unit);
            if (old_atb.interval == unit->GetATBTime() && old_atb.priority == unit->GetAttackPriority())
                continue;

            __PRINTF("战斗对象 %d 的ATB更新，new_atb_time: %d", unit->GetID(), unit->GetATBTime());

			ATBEntry new_atb;
			new_atb.interval = unit->GetATBTime();
			new_atb.priority = unit->GetAttackPriority();
			new_atb.unit = unit;
			if (old_atb.interval < unit->GetATBTime())
			{
				new_atb.tick = old_atb.tick + (unit->GetATBTime() - old_atb.interval) / MSEC_PER_TICK;
			}
			else if (old_atb.interval > unit->GetATBTime())
			{
				new_atb.tick = old_atb.tick - (old_atb.interval - unit->GetATBTime()) / MSEC_PER_TICK;
			}
			else if (old_atb.interval == unit->GetATBTime())
			{
				new_atb.tick = old_atb.tick;
			}

			//先删除老ATB
			atb_set_.erase(it_set);
			//再插入新ATB
			atb_set_.insert(new_atb);
		}
	}

	change_obj_vec_.clear();
}

void CombatATB::DoAttack()
{
	while (!ready_obj_list_.empty())
	{
		CombatUnit* __obj = ready_obj_list_.front();

        if (__obj->IsAlive())
        {
            cur_obj_ = __obj;
            bool ret = cur_obj_->StartAttack();
            cur_obj_ = NULL;

            if (!ret)
            {
                //施放技能失败
                //战斗可能暂停了, XP技能也会导致这种情况, 但是战斗未被暂停
                return;
            }

            combat_->SetAction(__obj, total_tick_);
        }

        //删除施放成功的ATB
        OBJList::iterator it = ready_obj_list_.begin();
        for (; it != ready_obj_list_.end(); ++ it)
        {
            if (*it == __obj)
            {
                ready_obj_list_.erase(it);
                break;
            }
        }

        //战斗被场景脚本暂停了
        if (!active_)
        {
            return;
        }

		//搜集即时ATB
		CollectInstantATB();

		/*cur_obj_ = __obj;
		bool ret = cur_obj_->StartAttack();
		cur_obj_ = NULL;

		if (!ret)
		{
            //施放技能失败
            //战斗可能暂停了, XP技能也会导致这种情况, 但是战斗未被暂停
            return;
        }

        if (ready_obj_list_.empty())
        {
            //本次攻击的ATB被提前删除了
            //没有等待的战斗对象了
            return;
        }

        combat_->SetAction(__obj, total_tick_);

        //删除施放成功的ATB
        OBJList::iterator it = ready_obj_list_.begin();
        for (; it != ready_obj_list_.end(); ++ it)
        {
            if (*it == __obj)
            {
                ready_obj_list_.erase(it);
                break;
            }
        }

        //战斗被场景脚本暂停了
        if (!active_)
        {
            return;
        }

		//搜集即时ATB
		CollectInstantATB();*/
	};

	//所有攻击均被成功处理
	assert(ready_obj_list_.size() == 0);
}

bool CombatATB::IsATBExist(UnitID unit_id) const
{
	ATBSet::const_iterator it_atb_set = std::find_if(atb_set_.begin(), atb_set_.end(), ATBFinder(unit_id));
	if (it_atb_set != atb_set_.end())
		return true;

	OBJList::const_iterator it_ready = std::find_if(ready_obj_list_.begin(), ready_obj_list_.end(), OBJFinder(unit_id));
	if (it_ready != ready_obj_list_.end())
		return true;

	OBJVec::const_iterator it_instant = std::find_if(instant_obj_vec_.begin(), instant_obj_vec_.end(), OBJFinder(unit_id));
	if (it_instant != instant_obj_vec_.end())
		return true;

	return false;
}

bool CombatATB::IsReadyATB(UnitID unit_id) const
{
	OBJList::const_iterator it = std::find_if(ready_obj_list_.begin(), ready_obj_list_.end(), OBJFinder(unit_id));
	return it != ready_obj_list_.end();
}

bool CombatATB::IsInstantATB(UnitID unit_id) const
{
	OBJVec::const_iterator it = std::find_if(instant_obj_vec_.begin(), instant_obj_vec_.end(), OBJFinder(unit_id));
	return it != instant_obj_vec_.end();
}

void CombatATB::Clear()
{
	atb_set_.clear();
	ready_obj_list_.clear();
	change_obj_vec_.clear();
	instant_obj_vec_.clear();

	cur_obj_ = NULL;
    active_  = false;
}

void CombatATB::Trace()
{
	__PRINTF("----------------------就绪战斗对象----------------------");
	OBJList::const_iterator it_obj = ready_obj_list_.begin();
	OBJList::const_iterator ie_obj = ready_obj_list_.end();
	for (; it_obj != ie_obj; ++ it_obj)
	{
		CombatUnit* unit = *it_obj;
		__PRINTF("战斗对象ID(%3d)", unit->GetID());
	}

	__PRINTF("----------------------预备战斗对象----------------------");
	ATBSet::const_iterator it_atb = atb_set_.begin();
	ATBSet::const_iterator ie_atb = atb_set_.end();
	for (; it_atb != ie_atb; ++ it_atb)
	{
		CombatUnit* unit = it_atb->unit;
		__PRINTF("战斗对象ID(%3d): 剩余TICK(%d).", unit->GetID(), it_atb->tick);
	}
}

};
