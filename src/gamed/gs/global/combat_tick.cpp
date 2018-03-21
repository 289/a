#include "combat_tick.h"

#include "shared/net/eventloop.h"

#include "game_module/combat/include/combat_header.h"


namespace gamed {

using namespace shared;
using namespace shared::net;

void CombatTick::TickCallback(void* pdata)
{
	// combat heartbeat
	combat::HeartBeat();
}

CombatTick::CombatTick()
	: evloop_thread_(NULL),
	  loop_(NULL)
{
}

CombatTick::~CombatTick()
{
}

void CombatTick::StartThread()
{
	ASSERT(NULL == evloop_thread_);
	evloop_thread_ = new EventLoopThread(BIND_MEM_CB(&CombatTick::LoopThreadRunStatusCB, this));
	evloop_thread_->StartLoop();
}

void CombatTick::StopThread()
{
	ASSERT(NULL != evloop_thread_);
	SAFE_DELETE(evloop_thread_);
}

// run in loop thread
void CombatTick::LoopThreadRunStatusCB(EventLoop* loop, bool is_start_run)
{
	if (is_start_run)
	{
		loop_     = loop;
		timer_id_ = loop_->RunEveryMsecs(kCombatTickTime, CombatTick::TickCallback, NULL);
	}
	else
	{
		loop_->Cancel(timer_id_);
		loop_ = NULL;
	}
}

} // namespace gamed

