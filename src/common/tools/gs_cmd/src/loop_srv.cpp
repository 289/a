#include "loop_srv.h"

#include "shared/base/base_define.h"
#include "shared/net/manager/network.h"
#include "shared/net/eventloopthread.h"
#include "shared/logsys/logging.h"

#include "gs_client.h"


namespace gsCmd {

using namespace shared;
using namespace shared::net;

LoopService::LoopService()
	: evloop_thread_(NULL),
	  loop_(NULL),
	  is_started_(false)
{
}

LoopService::~LoopService()
{
}

bool LoopService::Init()
{
	assert(evloop_thread_ == NULL);
	evloop_thread_ = new EventLoopThread(BIND_MEM_CB(&LoopService::LoopThreadRunStatusCB, this));
	assert(NULL != evloop_thread_);
	return true;
}

void LoopService::StartLoop()
{
	loop_ = evloop_thread_->StartLoop();
	NetWork::Init(loop_);
	is_started_ = true;
}

void LoopService::StopLoop()
{
	assert(is_started_);
	DeleteMemberInOrder();
}

void LoopService::LoopThreadRunStatusCB(shared::net::EventLoop* loop, bool is_start_run)
{
	if (is_start_run)
	{
		LOG_INFO << "LoopService thread start!";
	}
	else // thread stop
	{
		loop_ = NULL;
		s_pGSClient->Release();
		LOG_INFO << "LoopService thread stop!";
	}
}

void LoopService::DeleteMemberInOrder()
{
	SAFE_DELETE(evloop_thread_);
}

} // namespace gamed
