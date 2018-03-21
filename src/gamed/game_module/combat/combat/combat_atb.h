#ifndef __GAMED_MODULE_COMBAT_ATB_H__
#define __GAMED_MODULE_COMBAT_ATB_H__

#include <set>
#include <list>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <string>

#include "combat_types.h"

namespace combat
{

/**
 * @class: CombatATB
 * @brief: 战斗ATB暂停规则，ATB在3种情况下会被暂停：
 *         假设战斗中有玩家A和B;
 *         1) A的ATB时间到，向B发起攻击，但是此时A正在被别人攻击，则A暂停所有ATB直至A被攻击动作结束;
 *         2) A的ATB时间到，向B发起攻击，但是此时B正在攻击别人，则B暂停所有ATB直至B的攻击动作结束;
 *         3) 战斗场景脚本暂停ATB，场景脚本暂停ATB时ATB可能已经处于暂停状态;
 *
 *         注意：
 *         1) ATB被暂停后，所有的战斗对象都不能发起新的攻击，直至ATB重新恢复;
 *         2) ATB被暂停后，其它战斗对象无法再次暂停ATB;
 *
 * @brief  atb_set:  等待调度的战斗对象
 * @brief  readyATB: ATB时间到了的战斗对象
 * @brief  即时ATB： 需要立即施放技能的ATB，放到ready_obj_list_的最前面。
 * @brief  变化ATB： ATB时间发生变化的战斗对象，需要重新计算其在atb_set_中的位置。
 */

class Combat;
class CombatUnit;
class CombatATB
{
private:
	struct ATBEntry
	{
		int32_t interval;            // ATB-TIME(ms)
		int32_t priority;            // 攻击优先级
		mutable int32_t tick;        // ATB-TICK
		mutable int32_t remain_time; // 剩余ATB时间(ms)
		CombatUnit* unit;            // 战斗对象
	};

	struct ATBCMP
	{
		bool operator()(const ATBEntry& lhs, const ATBEntry& rhs)
		{
			if (lhs.remain_time < rhs.remain_time)
				return true;
			else if (lhs.remain_time == rhs.remain_time)
				return lhs.priority > rhs.priority;
			return false;
		}
	};

	typedef std::list<CombatUnit*> OBJList;
	typedef std::vector<CombatUnit*> OBJVec;
	typedef std::multiset<ATBEntry, ATBCMP> ATBSet;

	struct ATBFinder
	{
		UnitID unit_id;
		ATBFinder(UnitID id): unit_id(id) {}
		bool operator() (const ATBSet::value_type& v) const;
	};

private:
	ATBSet       atb_set_;             // ATB序列集合
	OBJList      ready_obj_list_;      // 等待施放技能的战斗对象
	OBJVec       change_obj_vec_;      // ATB发生变化的战斗对象
	OBJVec       instant_obj_vec_;     // 需要立即施放技能的战斗对象
	CombatUnit*  cur_obj_;             // 正在施放技能的战斗对象
	Combat*      combat_;              // 属于哪个战场
    bool         active_;              // 是否处于激活状态
    uint32_t     total_tick_;          // 当前进行过的总Tick数

public:
	explicit CombatATB(Combat* obj):
		cur_obj_(NULL),
		combat_(obj),
        active_(false),
        total_tick_(0)
	{ }

	~CombatATB()
	{
		atb_set_.clear();
		ready_obj_list_.clear();
		change_obj_vec_.clear();
		instant_obj_vec_.clear();

		cur_obj_ = NULL;
		combat_  = NULL;
        active_  = false;
        total_tick_ = 0;
	}

	void Init();
	void RegisterATB(CombatUnit* unit);
	void RegisterInstantATB(CombatUnit* unit);
	void UnRegisterATB(UnitID unit_id);
	void UnRegisterAllATB();
	void ResetATB(CombatUnit* unit);
	void Activate();
	void DeActivate();
	void OnATBChange(CombatUnit* unit);
	void HeartBeat();
	void Clear();
    inline uint32_t TotalTick() const;

private:
	bool IsATBExist(UnitID unit_id) const;
	bool IsReadyATB(UnitID unit_id) const;
	bool IsInstantATB(UnitID unit_id) const;
	void CollectReadyATB();
	void CollectInstantATB();
	void DoChange();
	void DoAttack();
	void Trace();
};

inline uint32_t CombatATB::TotalTick() const
{
    return total_tick_;
}

}; // namespace combat

#endif // __GAMED_MODULE_COMBAT_ATB_H__
