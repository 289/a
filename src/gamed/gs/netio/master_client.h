#ifndef GAMED_GS_NETWORK_MASTER_CLIENT_H_
#define GAMED_GS_NETWORK_MASTER_CLIENT_H_

#include "shared/base/callback_bind.h"
#include "shared/net/tcpclient.h"
#include "shared/net/protobuf/codec_proto.h"

#include "queue_msg_type.h"


namespace gamed {

using namespace std;
using namespace shared;
using namespace shared::net;

class MasterSession;
class MasterClient : public TcpClient
{
public:
	typedef bind::Callback<void (int32_t, const TcpConnection*, bool)> OnConnectedCallback;

	MasterClient(EventLoop* loop,
			     const InetAddress& server_addr,
			     const string& name,
				 TcpClient::NextConnIdCallback next_conn_id_cb,
				 const OnConnectedCallback& cb = bind::NullCallback());

	virtual ~MasterClient();

	// Not thread safe, but in loop
	virtual void NewConnection(int sockfd);
	// Not thread safe, but in loop
	virtual void RemoveConnection(TcpConnection* conn);

	inline bool  IsConnected() const { return NULL != connection_; }
	TcpConnection* connection() const { return TcpClient::connection();	}
	inline void  set_tcp_nodelay(bool on) { tcp_nodelay_ = on; }


private:
	OnConnectedCallback on_connected_cb_;
	bool tcp_nodelay_;
};


class MasterSession : public TcpConnection
{
	enum eStatusMask
	{
		STATUS_DISCONNECT = 0,
		STATUS_CONNECTED  = 0x0001,
	};

public:
	MasterSession(EventLoop* loop,
			      int32_t sid,
				  const std::string& name,
				  int sockfd,
				  const InetAddress& localAddr,
				  const InetAddress& peerAddr,
				  MasterClient& master_client);

	virtual ~MasterSession();

	virtual void OnConnected();
	virtual void OnRecv(Buffer* pbuf);
	virtual void OnClose();
	virtual void OnDestroy();


private:
	void    ClearMessageVec();
	void    set_status(eStatusMask status) { status_ = status; }

	static void CodecErrorCallback(shared::net::TcpConnection* conn,
                                   shared::net::Buffer* buf,
                                   ProtobufCodec::ErrorCode errorCode);


private:
	MasterClient&       master_client_;
	ProtobufCodec       codec_;

	eStatusMask	        status_;
	std::vector<ProtobufCodec::MessagePtr>  vec_messagesPtr_;
	std::vector<network::QueuedRecvMsg>     recvmsg_wrap_vec_;
};

} // namespace gamed

#endif // GAMED_GS_NETWORK_MASTER_CLIENT_H_
