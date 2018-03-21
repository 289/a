#ifndef GAMED_GS_OBJ_AIAGGRO_H_
#define GAMED_GS_OBJ_AIAGGRO_H_

#include "aggrolist.h"


namespace gamed {

struct AggroParam
{
	AggroParam();
	float aggro_range;
	int   aggro_time;
	int   faction;
	int   enemy_faction;
};

class A2DVECTOR;
class AIObject;

/**
 * @brief AggroPolicy 负责控制仇恨策略
 */
class AggroPolicy
{
public:
	AggroPolicy(AIObject* obj);
	virtual ~AggroPolicy();

	bool Init(const AggroParam& param);
	void Heartbeat();

	void ChangeAggroParam(const AggroParam& param);
	bool GetFirst(XID& target);
	bool AggroWatch(const XID& target, const A2DVECTOR& pos, int faction);
	bool ChangeWatch(const XID& target, const A2DVECTOR& pos, int faction);
	int  AddToFirst(const XID& id, int addon_rage);
	void Remove(const XID& id);
	void Clear();

	inline bool   CanWatch() const;
	inline bool   IsEmpty() const;
	inline size_t Size() const;
	inline void   SetAggroWatch(bool is_watch);
	inline void   RefreshTimer(const XID& target);
	inline float  GetAggroRange() const;


protected:
	int  AddAggro(const XID& id, int rage);


private:
	AIObject* self_;
	AggroList aggro_list_;      // 仇恨列表
	float     aggro_range_;     // 仇恨视野范围，单位为米
	float     aggro_max_range_; // 超过这个数则脱离仇恨，单位为米
	int       aggro_time_;      // 仇恨超时时间，超过这个值则脱离仇恨，单位为秒
	int       faction_;         // 阵营
	int       enemy_faction_;   // 敌对阵营
	int       timeout_timer_;   // 心跳超时计时器
	bool      aggro_watch_;     // 控制能否watch enemy
};

///
/// inline func
///
inline bool AggroPolicy::IsEmpty() const
{
	return aggro_list_.IsEmpty();
}

inline size_t AggroPolicy::Size() const
{
	return aggro_list_.Size();
}

inline void AggroPolicy::SetAggroWatch(bool is_watch)
{
	aggro_watch_ = is_watch;
}

inline bool AggroPolicy::CanWatch() const
{
	return aggro_watch_;
}

inline void AggroPolicy::RefreshTimer(const XID& target)
{
	if (aggro_list_.IsFirst(target))
	{
		timeout_timer_ = aggro_time_;
	}
}

} // namespace gamed

#endif // GAMED_GS_OBJ_AIAGGRO_H_
