#include "network_evloop.h"

#include "shared/logsys/logging.h"

#include "gs/netio/master_client.h"
#include "gs/netio/link_client.h"


namespace gamed {

namespace {

	bool CheckIpAndPort(const char* ip_str, int32_t port)
	{
		if (port <= 1024 || port > 65536)
			return false;

		if (!InetAddress::CheckIpAddress(ip_str))
			return false;

		return true;
	}

} // Anonymous

///
/// static members
///
AtomicInt32 NetworkEvLoop::link_next_conn_id_;
AtomicInt32 NetworkEvLoop::master_next_conn_id_;

///
/// static func
///
int32_t NetworkEvLoop::GetLinkNextConnId()
{
	return link_next_conn_id_.increment_and_get();
}
	
int32_t NetworkEvLoop::GetMasterNextConnId()
{
	return master_next_conn_id_.increment_and_get();
}

///
/// NetworkEvLoop
///
NetworkEvLoop::NetworkEvLoop()
	: evloop_thread_(NULL),
	  loop_(NULL),
	  is_inited_(false),
	  is_started_(false),
	  master_tcp_nodelay_(false),
	  link_tcp_nodelay_(false),
	  link_recv_queue_(NULL),
	  master_recv_queue_(NULL)
{
}

NetworkEvLoop::~NetworkEvLoop()
{
	DeleteMemberInOrder();

	for (size_t i = 0; i < master_addr_vec_.size(); ++i)
	{
		DELETE_SET_NULL(master_addr_vec_[i]);
	}
	master_addr_vec_.clear();

	for (size_t i = 0; i < link_addr_vec_.size(); ++i)
	{
		DELETE_SET_NULL(link_addr_vec_[i]);
	}
	link_addr_vec_.clear();
}

bool NetworkEvLoop::Init(LinkRecvQueue* link_recvqueue, MasterRecvQueue* master_recvqueue)
{
	assert(!is_inited_);
	if (NULL == link_recvqueue || NULL == master_recvqueue) return false;

	link_recv_queue_    = link_recvqueue;
	master_recv_queue_  = master_recvqueue;
	is_inited_          = true;

	return true;
}

bool NetworkEvLoop::Start()
{
	assert(is_inited_ && NULL == evloop_thread_);

	evloop_thread_ = new EventLoopThread(BIND_MEM_CB(&NetworkEvLoop::LoopThreadRunStatusCB, this));
	assert(NULL != evloop_thread_);

	evloop_thread_->StartLoop();

	is_started_ = true;
	return true;
}

void NetworkEvLoop::Stop()
{
	assert(is_started_);
	DeleteMemberInOrder();
}

void NetworkEvLoop::LoopThreadRunStatusCB(EventLoop* loop, bool is_start_run)
{
	if (is_start_run)
	{
		loop_ = loop;

		// master client
		assert(master_addr_vec_.size() >= 1);
		for (size_t i = 0; i < master_addr_vec_.size(); ++i)
		{
			string name    = string("MasterServer: ") + master_addr_vec_[i]->ToIpPort();
			MasterClient* pmasterclient = new MasterClient(loop, 
					                                       *master_addr_vec_[i], 
														   name, 
														   GetMasterNextConnId,
					                                       BIND_MEM_CB(&NetworkEvLoop::MasterConnStatusCB, this));
			pmasterclient->set_tcp_nodelay(master_tcp_nodelay_);
			pmasterclient->Connect(); // queue in loop, run after RunLoop()
			master_client_vec_.push_back(pmasterclient);
		}

		// link client
		assert(link_addr_vec_.size() >= 1);
		for (size_t i = 0; i < link_addr_vec_.size(); ++i)
		{
			string name = string("LinkServer: ") + link_addr_vec_[i]->ToIpPort();
			LinkClient* plinkclient = new LinkClient(loop, 
					                                 *link_addr_vec_[i], 
													 name,
													 GetLinkNextConnId,
					                                 BIND_MEM_CB(&NetworkEvLoop::LinkConnStatusCB, this));
			plinkclient->set_tcp_nodelay(link_tcp_nodelay_);
			plinkclient->Connect(); // queue in loop, run after RunLoop()
			link_client_vec_.push_back(plinkclient);
		}
	}
	else // thread stop
	{
		loop_ = NULL;

		// master client
		for (size_t i = 0; i < master_client_vec_.size(); ++i)
		{
			DELETE_SET_NULL(master_client_vec_[i]);
		}
		master_client_vec_.clear();

		// link client
		for (size_t i = 0; i < link_client_vec_.size(); ++i)
		{
			DELETE_SET_NULL(link_client_vec_[i]);
		}
		link_client_vec_.clear();
	}
}

int NetworkEvLoop::AddNewLink(const std::string& ip, uint16_t port)
{
	if (loop_ == NULL || !CheckIpAndPort(ip.c_str(), port))
		return ANC_ERR_PARAM;

	InetAddress tmpAddr(ip, port);
	for (size_t i = 0; i < link_addr_vec_.size(); ++i)
	{
		if (link_addr_vec_[i]->ToIpPort() == tmpAddr.ToIpPort())
		{
			return ANC_ERR_CONNECTED;
		}
	}

	InetAddress* netAddr = new InetAddress(ip, port);
	link_addr_vec_.push_back(netAddr);
	string name = string("LinkServer: ") + netAddr->ToIpPort();
	LinkClient* plinkclient = new LinkClient(loop_, 
			                                 *netAddr, 
			                                 name,
			                                 GetLinkNextConnId,
			                                 BIND_MEM_CB(&NetworkEvLoop::LinkConnStatusCB, this));
	plinkclient->set_tcp_nodelay(link_tcp_nodelay_);
	plinkclient->Connect(); // queue in loop, run after RunLoop()
	link_client_vec_.push_back(plinkclient);
	return ANC_SUCCESS;
}

int NetworkEvLoop::AddNewMaster(const std::string& ip, uint16_t port)
{
	if (loop_ == NULL || !CheckIpAndPort(ip.c_str(), port))
		return ANC_ERR_PARAM;

	InetAddress tmpAddr(ip, port);
	for (size_t i = 0; i < master_addr_vec_.size(); ++i)
	{
		if (master_addr_vec_[i]->ToIpPort() == tmpAddr.ToIpPort())
		{
			return ANC_ERR_CONNECTED;
		}
	}

	InetAddress* netAddr = new InetAddress(ip, port);
	master_addr_vec_.push_back(netAddr);
	string name = string("MasterServer: ") + netAddr->ToIpPort();
	MasterClient* pmasterclient = new MasterClient(loop_, 
			                                       *netAddr, 
			                                       name, 
			                                       GetMasterNextConnId,
			                                       BIND_MEM_CB(&NetworkEvLoop::MasterConnStatusCB, this));
	pmasterclient->set_tcp_nodelay(master_tcp_nodelay_);
	pmasterclient->Connect(); // queue in loop, run after RunLoop()
	master_client_vec_.push_back(pmasterclient);
	return ANC_SUCCESS;
}

// locked by caller
void NetworkEvLoop::DeleteMemberInOrder()
{
	SAFE_DELETE(evloop_thread_);
}

void NetworkEvLoop::MasterConnStatusCB(int32_t sid, const TcpConnection* pconn, bool is_onconnect)
{
	loop_->AssertInLoopThread();

	RWLockWriteGuard wrlock(master_rwlock_);
	if (is_onconnect)
	{
		std::pair<MasterSessionMap::iterator, bool> ret;
		ret = master_session_map_.insert(std::pair<int32_t, const TcpConnection*>(sid, pconn));
		assert(ret.second);
	}
	else
	{
		size_t n = master_session_map_.erase(sid);
		(void)n; assert(n == 1);
	}
}

void NetworkEvLoop::LinkConnStatusCB(int32_t sid, const TcpConnection* pconn, bool is_onconnect)
{
	loop_->AssertInLoopThread();

	RWLockWriteGuard wrlock(link_rwlock_);
	if (is_onconnect)
	{
		std::pair<LinkSessionMap::iterator, bool> ret;
		ret = link_session_map_.insert(std::pair<int32_t, const TcpConnection*>(sid, pconn));
		assert(ret.second);
	}
	else
	{
		size_t n = link_session_map_.erase(sid);
		(void)n; assert(n == 1);
	}
}

void NetworkEvLoop::SendToMaster(int32_t sid, const ProtobufCodec::MessageRef msg_ref)
{
	RWLockReadGuard rdlock(master_rwlock_);
	MasterSessionMap::iterator it = master_session_map_.find(sid);
	if (master_session_map_.end() != it)
	{
		ProtobufCodec::SendMsg(const_cast<TcpConnection*>(it->second), msg_ref);
	}
	else
	{
		LOG_ERROR << "Master[" << sid << "] not found!";
	}
}

void NetworkEvLoop::SendToLink(int32_t sid, const ProtobufCodec::MessageRef msg_ref)
{
	RWLockReadGuard rdlock(link_rwlock_);
	LinkSessionMap::iterator it = link_session_map_.find(sid);
	if (link_session_map_.end() != it)
	{
		ProtobufCodec::SendMsg(const_cast<TcpConnection*>(it->second), msg_ref);
	}
	else
	{
		LOG_ERROR << "Link[" << sid << "] not found!";
	}
}

} // namespace gamed
