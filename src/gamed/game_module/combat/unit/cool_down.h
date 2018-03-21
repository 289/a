#ifndef __GAME_MODULE_COMBAT_COOL_DOWN_H__
#define __GAME_MODULE_COMBAT_COOL_DOWN_H__

#include <set>

namespace combat
{

///
/// ��ȴʱ�䶨��, ��λ��ms
///
//#define CD_TIME_PET_ATTACK 1000


///
/// ��ȴINDEX
///
enum COOL_DOWN_INDEX
{
	COOL_DOWN_INDEX_INVALID,
	COOL_DOWN_INDEX_PET_ATTACK,
	COOL_DOWN_INDEX_MAX,
};


/**
 * @class CoolDownMan
 * @brief ��ȴʱ�������
 */
class CoolDownMan
{
private:
	typedef int CoolDownID;

	struct CDEntry
	{
		CoolDownID cooldown_id;  //��ȴID
		int32_t    interval;     //��ȴʱ��(ms)
		mutable int32_t timeout; //��ȴTICK

		CDEntry(CoolDownID id, int32_t cool_down_time): cooldown_id(id), interval(cool_down_time)
		{
			timeout = cool_down_time / MSEC_PER_TICK;
		}
	};

	struct CDCMP
	{
		bool operator() (const CDEntry& lhs, const CDEntry& rhs) const
		{
			return lhs.timeout < rhs.timeout;
		}
	};

	struct CDFinder
	{
		CoolDownID cd_id;
		CDFinder(CoolDownID id): cd_id(id) {}
		bool operator() (const CDEntry& entry) const
		{
			return cd_id == entry.cooldown_id;
		}
	};

	typedef std::set<CDEntry, CDCMP> CoolDownSet;
	CoolDownSet cd_set_;

public:
	bool TestCoolDown(CoolDownID id) const
	{
		CoolDownSet::const_iterator it = std::find_if(cd_set_.begin(), cd_set_.end(), CDFinder(id));
		return it == cd_set_.end();
	}

	bool SetCoolDown(CoolDownID id, int interval)
	{
		if (interval <= MSEC_PER_TICK)
			return false;

		CoolDownSet::iterator it = std::find_if(cd_set_.begin(), cd_set_.end(), CDFinder(id));
		if (it != cd_set_.end())
			return false;

		CDEntry entry(id, interval);
		cd_set_.insert(entry);
		return true;
	}

	bool ClrCoolDown(CoolDownID id)
	{
		CoolDownSet::iterator it = std::find_if(cd_set_.begin(), cd_set_.end(), CDFinder(id));
		if (it == cd_set_.end())
			return false;

		cd_set_.erase(it);
		return true;
	}

	void HeartBeat()
	{
		CoolDownSet::iterator it = cd_set_.begin();
		while (it != cd_set_.end())
		{
			it->timeout -= 1;
			if (it->timeout <= 0)
			{
				cd_set_.erase(it ++);
			}
			else
			{
				++ it;
			}
		}
	}

	void Clear()
	{
		cd_set_.clear();
	}
};

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_COOL_DOWN_H__
