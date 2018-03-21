#ifndef GAMED_GS_GLOBAL_COMBAT_TICK_H_
#define GAMED_GS_GLOBAL_COMBAT_TICK_H_

#include "shared/base/singleton.h"
#include "shared/base/base_define.h"

#include "shared/net/eventloopthread.h"
#include "shared/net/timerid.h"


namespace gamed {

class CombatTick : public shared::Singleton<CombatTick>
{
	friend class shared::Singleton<CombatTick>;
public:
	static inline CombatTick* GetInstance() {
		return &(get_mutable_instance());
	}

	void    StartThread();
	void    StopThread();


protected:
	CombatTick();
	~CombatTick();


private:
	static void TickCallback(void*);
	void   LoopThreadRunStatusCB(shared::net::EventLoop* loop, bool is_start_run);


private:
	static const int32_t kCombatTickTime = 100; // Units are millisecond
	static const int32_t kTickPerSecond  = 1000 / kCombatTickTime;

	shared::net::EventLoopThread*  evloop_thread_;
	shared::net::EventLoop*        loop_;
	shared::net::TimerId           timer_id_;

};

#define s_pCombatTick gamed::CombatTick::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_COMBAT_TICK_H_
