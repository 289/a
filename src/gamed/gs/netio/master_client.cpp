#include "master_client.h"

#include <stdio.h> // snprintf

#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"
#include "shared/net/sockets_ops.h"

#include "network_evloop.h"
#include "netio_if.h"

// protocol
#include "common/protocol/gen/global/connection_status.pb.h"


using namespace gamed::network;
using namespace common::protocol;

namespace gamed {

MasterClient::MasterClient(EventLoop* loop,
			               const InetAddress& server_addr,
			               const string& name,
						   TcpClient::NextConnIdCallback next_conn_id_cb,
						   const OnConnectedCallback& cb)
	: TcpClient(loop, server_addr, name, next_conn_id_cb),
	  on_connected_cb_(cb),
	  tcp_nodelay_(false)
{ 
	EnableRetry();
}

MasterClient::~MasterClient()
{
}

void MasterClient::NewConnection(int sockfd)
{
	MutexLockGuard lock(mutex_);

	loop_->AssertInLoopThread();
	InetAddress peerAddr(sockets::get_peer_addr(sockfd));
	char buf[32];
	int32_t tmpconnid = GetNextConnId();
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.ToIpPort().c_str(), tmpconnid);
	string connName = name_ + buf;

	InetAddress localAddr(sockets::get_local_addr(sockfd));
	// FIXME poll with zero timeout to double confirm the new connection
	// FIXME use make_shared if necessary
	MasterSession* conn = new MasterSession(loop_,
			                                tmpconnid,
				                            connName,
				                            sockfd,
				                            localAddr,
				                            peerAddr,
											*this);
	connection_ = static_cast<TcpConnection*>(conn);
	conn->SetTcpNoDelay(tcp_nodelay_);

	if (on_connected_cb_) {
		on_connected_cb_(tmpconnid, static_cast<TcpConnection*>(conn), true);
	}

	LOG_INFO << "MasterClient::connector[" << name_ << "] is connected, server addr - "
		        << connector_->server_address().ToIpPort();
	
	conn->ConnectEstablished();
}

void MasterClient::RemoveConnection(TcpConnection* conn)
{
	{
		MutexLockGuard lock(mutex_);
		LOG_ERROR << "Master connection remove, MasterServer addr:" << conn->peer_address().ToIpPort(); 

		if (on_connected_cb_) {
			on_connected_cb_(conn->get_conn_id(), NULL, false);
		}
	}

	TcpClient::RemoveConnection(conn);
}


///
/// MasterSession
///
void MasterSession::CodecErrorCallback(shared::net::TcpConnection* conn,
                                     shared::net::Buffer* buf,
                                     ProtobufCodec::ErrorCode errorCode)
{
	LOG_ERROR << "MasterSession::CodecErrorCallback - " << ProtobufCodec::ErrorCodeToString(errorCode);
	if (conn && conn->connected())
	{
		conn->Shutdown();
	}
	assert(false);
}

MasterSession::MasterSession(EventLoop* loop,
		                     int32_t sid,
				             const std::string& name,
				             int sockfd,
				             const InetAddress& localAddr,
				             const InetAddress& peerAddr,
							 MasterClient& master_client)
	: TcpConnection(loop, sid, name, sockfd, localAddr, peerAddr),
	  master_client_(master_client),
	  codec_(MasterSession::CodecErrorCallback),
	  status_(STATUS_DISCONNECT)
{ 
}

MasterSession::~MasterSession()
{
	set_status(STATUS_DISCONNECT);
	ClearMessageVec();
}

void MasterSession::ClearMessageVec()
{
    std::vector<ProtobufCodec::MessagePtr>::iterator it = vec_messagesPtr_.begin();
	for (; it != vec_messagesPtr_.end(); ++it) 
	{
		SAFE_DELETE(*it);
	}
	vec_messagesPtr_.clear();
}

void MasterSession::OnConnected()
{
	set_status(STATUS_CONNECTED);

	global::ConnectEstablished* notify_connected = new global::ConnectEstablished();
	notify_connected->set_type(global::ConnectionInfo::MASTER_CONN);

	QueuedRecvMsg tmp(get_conn_id(), notify_connected);
	s_pNetworkEvLoop->PutMasterProtobufMsg(tmp);
}

void MasterSession::OnClose()
{
	set_status(STATUS_DISCONNECT);
	master_client_.RemoveConnection(this);

	global::ConnectDestroyed* notify_closed = new global::ConnectDestroyed();
	notify_closed->set_type(global::ConnectionInfo::MASTER_CONN);
	notify_closed->set_masterid(NetIO::GetMasterIdBySid(get_conn_id()));

	QueuedRecvMsg tmp(get_conn_id(), notify_closed);
	s_pNetworkEvLoop->PutMasterProtobufMsg(tmp);
}

void MasterSession::OnDestroy()
{
	set_status(STATUS_DISCONNECT);

	global::ConnectDestroyed* notify_closed = new global::ConnectDestroyed();
	notify_closed->set_type(global::ConnectionInfo::MASTER_CONN);
	notify_closed->set_masterid(NetIO::GetMasterIdBySid(get_conn_id()));

	QueuedRecvMsg tmp(get_conn_id(), notify_closed);
	s_pNetworkEvLoop->PutMasterProtobufMsg(tmp);
}

void MasterSession::OnRecv(Buffer* pbuf)
{
	codec_.ParseMessages(static_cast<TcpConnection*>(this), pbuf, vec_messagesPtr_);

	for (size_t i = 0; i < vec_messagesPtr_.size(); ++i)
	{
		QueuedRecvMsg tmp(get_conn_id(), vec_messagesPtr_[i]);
		recvmsg_wrap_vec_.push_back(tmp);
	}
	vec_messagesPtr_.clear();

	// to queue
	s_pNetworkEvLoop->PutMasterProtobufMsg(recvmsg_wrap_vec_);
	recvmsg_wrap_vec_.clear();
}

} // namespace gamed
