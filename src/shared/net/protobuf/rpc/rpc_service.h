#ifndef SHARED_NET_PROTOBUF_RPC_SERVICE_H_
#define SHARED_NET_PROTOBUF_RPC_SERVICE_H_

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/singleton.h"

#include "shared/net/timed_task.h"
#include "shared/net/protobuf/rpc/rpc_channel.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "shared/net/protobuf/rpc/rpc_sendtunnel.h"


namespace google {
namespace protobuf {

	class Service;

}  // namespace protobuf
}  // namespace google


namespace shared {
namespace net {

namespace rpc 
{
	// delete period
	const int kPeriodDeleteCompletedRpcSecs = 2*60; // 5 minutes
	// timeout
	const int kRpcCallDefaultTimeoutSecs    = 30;   // 30 seconds
	const std::string kTimeoutErrorText     = "Timeout";

} // namespace rpc

class RpcMessage;


///
/// rpc in calling side (invoking services)
///
class RpcClient : noncopyable
{
	static const int32_t kTimeoutDefaultInterval = 600;
public:
	// RpcClient使用时推荐将其作为一个成员变量，如果做成全局变量要十分谨慎
	// 因为回调成员函数时，这个成员的this指针已经失效
	RpcClient();

	// RpcClient 提供两种方式进行Timeout检查，如果RpcClient和Loop在同一个线程
	// 可以传入一个loop进行回调,如果不在一个线程则请使用TimeoutHeartbeat()函数处理超时
	RpcClient(int64_t caller_id, int32_t caller_type, const RpcSendTunnelPtr& tunnel);
	RpcClient(EventLoop* loop,
			  int64_t caller_id, 
			  int32_t caller_type, 
			  const RpcSendTunnelPtr& tunnel);

	~RpcClient();

	void SetSendTunnel(const RpcSendTunnelPtr& tunnel);

	// must invoke every second
	// 该函数需要放在调用者Heartbeat()函数的第一行,早于其他模块的heartbeat
	void TimeoutHeartbeat(Timestamp cur_time);

	void OnRecvRpcMessage(const RpcMessage& message);

	template<typename Class>
	ServiceStub* CreateStub(const google::protobuf::Message& request);

	template<typename Class>
	void RpcCall(int32_t rpc_server_sid,
			     ServiceStub* pstub, 
			     const typename RpcCallbackT<Class>::RpcMessageTCallback& callback,
				 int32_t timeout_secs = rpc::kRpcCallDefaultTimeoutSecs);

	inline int64_t caller_id() const { return channel_.get_caller_id(); }


private:
	void TimerCheckCompletedRpcStub(const TimedTask* p);
	void CheckCompletedRpcStub();

	MutexLock          mutex_;
	RpcChannel         channel_;
	std::vector<ServiceStub*> stub_vec_;
	int32_t            timeout_counter_;
	TimedTaskManager*  timer_;
};

///
/// template function
///
template<typename Class>
ServiceStub* RpcClient::CreateStub(const google::protobuf::Message& request)
{
	Class* pstub = new Class(&channel_);
	bool ret = pstub->request().ParseFromString(request.SerializeAsString());
	assert(ret);

	return static_cast<ServiceStub*>(pstub);
}

template<typename Class>
void RpcClient::RpcCall(int32_t rpc_server_sid,
		                ServiceStub* pstub, 
		                const typename RpcCallbackT<Class>::RpcMessageTCallback& callback,
		                int32_t timeout_secs)
{
	if (pstub)
	{
		MutexLockGuard lock(mutex_);

		RpcCallback* pcallback = static_cast<RpcCallback*>(new RpcCallbackT<Class>(callback, pstub));
		pstub->registerCall(rpc_server_sid, pcallback, timeout_secs);

		stub_vec_.push_back(pstub);

		if (NULL == timer_)
		{
			timeout_counter_ = channel_.GetNextTimeoutSecs(Timestamp::Now(), kTimeoutDefaultInterval);
		}
	}
}


///
/// rpc in services provider side
///
class RpcServer : public shared::Singleton<RpcServer>
{
	friend class shared::Singleton<RpcServer>;
public:
	// 使用RpcServer一定要通过SetSendTunnel()指定发送通道
	static inline RpcServer* GetInstance() {
		return &(get_mutable_instance());
	}

	void SetSendTunnel(const RpcSendTunnelPtr& tunnel);

	void registerService(::google::protobuf::Service*);
	void registerFinalize();

	// thread safe
	void OnRecvRpcMessage(int32_t sendback_sid,
			              const RpcMessage& message);

protected:
	RpcServer();
	~RpcServer();


private:
	RpcChannel         channel_; // no timer
	typedef std::map<std::string, ::google::protobuf::Service*> ServicesMap;
	ServicesMap        services_;
	MutexLock          mutex_;
};

// instance
#define s_pRpcServer shared::net::RpcServer::GetInstance()

} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_RPC_SERVICE_H_
