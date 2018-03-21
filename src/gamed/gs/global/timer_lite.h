#ifndef GAMED_GS_GLOBAL_TIMER_LITE_H_
#define GAMED_GS_GLOBAL_TIMER_LITE_H_

#include <stdint.h>
#include <assert.h>
#include <set>
#include <map>
#include <vector>
#include <limits>

#include "shared/base/noncopyable.h"


namespace gamed {

/**
 * @brief TimerLite
 * （1）该类是线程不安全的
 * （2）具体用法可以参考副本：scene/instance/ins_world_man.h
 */
template <typename T>
class TimerLite : shared::noncopyable
{
public:
	typedef std::pair<int64_t, T*> Entry;

	TimerLite();
	~TimerLite();

	// **** thread unsafe ****
	// 以下函数返回值都是下一个检查点的时间，单位与设入的值一致
	int64_t GetExpired(int64_t now, std::vector<Entry>& expired);
	int64_t AddEntry(int64_t when, T* ent); // 如果ent已经存在，则更新只when
	int64_t Cancel(T* ent);


private:
	typedef std::set<Entry> TimerSet;
	typedef std::map<T*, int64_t> ActiveTimerMap;
	int64_t GetNextTime() const;


private:
	// Timer set sorted by expiration
	TimerSet        timers_;
	ActiveTimerMap  active_timers_;
};

///
/// template func
///
template <typename T>
TimerLite<T>::TimerLite()
{
}

template <typename T>
TimerLite<T>::~TimerLite()
{
	timers_.clear();
	active_timers_.clear();
}

template <typename T>
int64_t TimerLite<T>::GetNextTime() const
{
	if (timers_.empty())
	{
		return std::numeric_limits<int64_t>::max();
	}
	return timers_.begin()->first;
}

template <typename T>
int64_t TimerLite<T>::GetExpired(int64_t now, std::vector<Entry>& expired)
{
	assert(timers_.size() == active_timers_.size());
	Entry sentry(now, reinterpret_cast<T*>(std::numeric_limits<intptr_t>::max()));
	//reinterpret_cast<T*>(UINTPTR_MAX));
	typename TimerSet::iterator end = timers_.lower_bound(sentry);
	assert(end == timers_.end() || now < end->first);
	std::copy(timers_.begin(), end, back_inserter(expired));
	timers_.erase(timers_.begin(), end);

	for (typename std::vector<Entry>::iterator it = expired.begin(); 
			it != expired.end(); ++it)
	{
		size_t n = active_timers_.erase(it->second);
		assert(n == 1); (void)n;
	}
	assert(timers_.size() == active_timers_.size());
	return GetNextTime();
}

template <typename T>
int64_t TimerLite<T>::AddEntry(int64_t when, T* ent)
{
	assert(timers_.size() == active_timers_.size());
	typename ActiveTimerMap::iterator it = active_timers_.find(ent);	
	if (it == active_timers_.end())
	{
		// not found
		active_timers_[ent] = when;

		std::pair<typename TimerSet::iterator, bool> result
			= timers_.insert(Entry(when, ent));
		assert(result.second); (void)result;
	}
	else // found
	{
		size_t n = timers_.erase(Entry(it->second, ent));
		assert(n == 1); (void)n;
		it->second = when;

		std::pair<typename TimerSet::iterator, bool> result
			= timers_.insert(Entry(when, ent));
		assert(result.second); (void)result;
	}
	assert(timers_.size() == active_timers_.size());
	return GetNextTime();
}

template <typename T>
int64_t TimerLite<T>::Cancel(T* ent)
{
	assert(timers_.size() == active_timers_.size());
	typename ActiveTimerMap::iterator it = active_timers_.find(ent);	
	if (it != active_timers_.end())
	{
		// found
		size_t n = timers_.erase(Entry(it->second, ent));
		assert(n == 1); (void)n;

		// map
		active_timers_.erase(it);
	}
	assert(timers_.size() == active_timers_.size());
	return GetNextTime();
}

} // namespace gamed

#endif // GAMED_GS_GLOBAL_TIMER_LITE_H_
