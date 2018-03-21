#ifndef GAMED_GS_NETWORK_NETWORK_EVLOOP_H_ 
#define GAMED_GS_NETWORK_NETWORK_EVLOOP_H_

#include "shared/base/singleton.h"
#include "shared/net/timed_task.h"
#include "shared/net/eventloopthread.h"
#include "shared/net/protobuf/codec_proto.h"

#include "queue_msg_type.h"


namespace gamed {

using namespace shared;
using namespace shared::net;
using namespace gamed::network;

class MasterClient;
class MasterSession;
class LinkClient;
class LinkSession;

/**
 * @brief 不要直接使用NetworkEvLoop类，网络接口是netio_if.h
 */
class NetworkEvLoop : public shared::Singleton<NetworkEvLoop>
{
	friend class shared::Singleton<NetworkEvLoop>;
	friend class MasterSession;
	friend class LinkSession;

public:
	enum CONN_STATUS
	{
		IO_CONNECTED = 0,
		IO_DISCONNECTED,
		IO_HEARTBEAT_TIMEOUT,
	};

	enum ADD_NEW_CONN
	{
		ANC_SUCCESS = 0,
		ANC_ERR_PARAM,
		ANC_ERR_CONNECTED,
	};


public:
	static inline NetworkEvLoop* GetInstance() {
		return &(get_mutable_instance());
	}

	// **** thread unsafe ****
	bool    Init(LinkRecvQueue* link_recvqueue, MasterRecvQueue* master_recvqueue);

	/**
	 * @brief Start 
	 *    1.调用Start前需要调用set_master_addr(),set_link_addr()设置link和master的地址
	 *      还需要调用Init()初始化接收队列.
	 */
	bool    Start();

	void    Stop();

	// ---- thread safe ----
	void    SendToMaster(int32_t sid, const ProtobufCodec::MessageRef msg_ref);
	void    SendToLink(int32_t sid, const ProtobufCodec::MessageRef msg_ref);
	// 取出的数据是new出来的，处理完毕后需要delete
	inline void    TakeAllLinkData(std::deque<QueuedRecvMsg>& queue);
	// blocking func
	inline void    TakeAllMasterData(std::deque<QueuedRecvMsg>& queue);

	inline bool    HasRecvLinkData() const;

	// **** thread unsafe ****
	int     AddNewLink(const std::string& ip, uint16_t port);
	int     AddNewMaster(const std::string& ip, uint16_t port);

	// **** thread unsafe ****
	inline void    set_master_addr(std::string ip, uint16_t port);
	inline void    set_link_addr(std::string ip, uint16_t port);
	inline void    set_master_tcp_nodelay(bool on);
	inline void    set_link_tcp_nodelay(bool on);


protected:
	NetworkEvLoop();
	~NetworkEvLoop();

	// 插入队列的 msg 必须是new出来的,在处理线程里process完毕后delete
	inline void PutLinkProtobufMsg(std::vector<QueuedRecvMsg>& msg_vec);
	inline void PutLinkProtobufMsg(QueuedRecvMsg msg);
	inline void PutMasterProtobufMsg(std::vector<QueuedRecvMsg>& msg_vec);
	inline void PutMasterProtobufMsg(QueuedRecvMsg msg);

	void    LoopThreadRunStatusCB(EventLoop* loop, bool is_start_run);

	void    LinkConnStatusCB(int32_t sid, const TcpConnection* pconn, bool is_onconnect);
	void    MasterConnStatusCB(int32_t sid, const TcpConnection* pconn, bool is_onconnect);


private:
	void    DeleteMemberInOrder();
	static int32_t GetLinkNextConnId();
	static int32_t GetMasterNextConnId();


private:
	EventLoopThread*   evloop_thread_;
	EventLoop*         loop_;
	RWLock             link_rwlock_;
	RWLock             master_rwlock_;
	bool               is_inited_;
	bool               is_started_;

	std::vector<MasterClient*>  master_client_vec_;
	std::vector<InetAddress*>   master_addr_vec_;
	typedef std::map<int32_t, const TcpConnection*> MasterSessionMap;
	MasterSessionMap   master_session_map_;
	bool               master_tcp_nodelay_;

	std::vector<LinkClient*>    link_client_vec_;
	std::vector<InetAddress*>   link_addr_vec_;
	typedef std::map<int32_t, const TcpConnection*> LinkSessionMap;
	LinkSessionMap     link_session_map_;
	bool               link_tcp_nodelay_;

	LinkRecvQueue*     link_recv_queue_;
	MasterRecvQueue*   master_recv_queue_;

	static AtomicInt32 link_next_conn_id_;
	static AtomicInt32 master_next_conn_id_;
};

///
/// inline func
///
inline void NetworkEvLoop::PutLinkProtobufMsg(std::vector<QueuedRecvMsg>& msg_vec)
{
	link_recv_queue_->putAll(msg_vec);
}

inline void NetworkEvLoop::PutLinkProtobufMsg(QueuedRecvMsg msg)
{
	link_recv_queue_->put(msg);
}

inline void NetworkEvLoop::PutMasterProtobufMsg(std::vector<QueuedRecvMsg>& msg_vec)
{
	master_recv_queue_->putAll(msg_vec);
}

inline void NetworkEvLoop::PutMasterProtobufMsg(QueuedRecvMsg msg)
{
	master_recv_queue_->put(msg);
}

inline bool NetworkEvLoop::HasRecvLinkData() const 
{
	return !link_recv_queue_->empty();
}

inline void NetworkEvLoop::TakeAllLinkData(std::deque<QueuedRecvMsg>& queue)
{
	link_recv_queue_->takeAll(queue);
}

// blocking function
inline void NetworkEvLoop::TakeAllMasterData(std::deque<QueuedRecvMsg>& queue)
{
	master_recv_queue_->takeAll(queue);
}

inline void NetworkEvLoop::set_master_addr(std::string ip, uint16_t port)
{
	assert(NULL == loop_); // assert loop-thread not start yet!
	master_addr_vec_.push_back(new InetAddress(ip, port));
}

inline void NetworkEvLoop::set_link_addr(std::string ip, uint16_t port)
{
	assert(NULL == loop_); // assert loop-thread not start yet!
	link_addr_vec_.push_back(new InetAddress(ip, port));
}

inline void NetworkEvLoop::set_master_tcp_nodelay(bool on)
{
	master_tcp_nodelay_ = on;
}

inline void NetworkEvLoop::set_link_tcp_nodelay(bool on)
{
	link_tcp_nodelay_ = on;
}


///
/// singleton
///
#define s_pNetworkEvLoop gamed::NetworkEvLoop::GetInstance()

} // namespace gamed

#endif // GAMED_GS_NETWORK_NETWORK_EVLOOP_H_
