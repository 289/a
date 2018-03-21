#include "loop_srv.h"

#include "shared/base/base_define.h"
#include "shared/net/manager/init.h"
#include "shared/net/manager/network.h"
#include "shared/net/eventloopthread.h"
#include "shared/logsys/logging.h"

#include "gs/global/loop_srv/cmd_server.h"


namespace gamed {

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

bool LoopService::Init(const shared::Conf& conf, int port_delta)
{
	assert(evloop_thread_ == NULL);
	assert(conf.is_inited());

	evloop_thread_ = new EventLoopThread(BIND_MEM_CB(&LoopService::LoopThreadRunStatusCB, this));
	assert(NULL != evloop_thread_);

	conf_       = conf;
    port_delta_ = port_delta;
	init::SetConf(conf);
	return true;
}

void LoopService::StartLoop()
{
	if (evloop_thread_->StartLoop() != NULL)
	{
		is_started_ = true;
	}
	else
	{
		LOG_FATAL << "LoopService StartLoop() error!";
	}
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
		loop_ = loop;
		LOG_INFO << "LoopService thread start!";
		StartServiceModule();
	}
	else // thread stop
	{
		loop_ = NULL;
		LOG_INFO << "LoopService thread stop!";
		StopServiceModule();
	}
}

void LoopService::DeleteMemberInOrder()
{
	SAFE_DELETE(evloop_thread_);
}

void LoopService::StartServiceModule()
{
	ASSERT(loop_ != NULL);
	NetWork::Init(loop_);
	
	// start service module
	std::string tmpstr = conf_.find(s_pCmdServer->Identification(), "address");
	if (!tmpstr.empty())
	{
        int port = port_delta_ + conf_.get_int_value(s_pCmdServer->Identification(), "port");
		NetWork::Server(s_pCmdServer, tmpstr.c_str(), port);
	}
}

void LoopService::StopServiceModule()
{
	// stop service module
	s_pCmdServer->Release();
}

} // namespace gamed
