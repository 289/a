#ifndef SHARED_NET_PROTOBUF_RPC_CHANNEL_H_
#define SHARED_NET_PROTOBUF_RPC_CHANNEL_H_

#include <google/protobuf/service.h>

#include <string>
#include <map>
#include <set>

#include "shared/base/timestamp.h"
#include "shared/base/atomic.h"
#include "shared/base/mutex.h"

#include "shared/net/protobuf/rpc/rpc.pb.h"
#include "shared/net/protobuf/rpc/rpc_sendtunnel.h"


// Service and RpcChannel classes are incorporated from
// google/protobuf/service.h

namespace google {
namespace protobuf {

// Defined in other files.
class Descriptor;            // descriptor.h
class ServiceDescriptor;     // descriptor.h
class MethodDescriptor;      // descriptor.h
class Message;               // message.h

class Closure;

class RpcController;
class Service;

}  // namespace protobuf
}  // namespace google


namespace shared {
namespace net {

class TimedTask;
class TimedTaskManager;
class RpcMessage;

// Abstract interface for an RPC channel.  An RpcChannel represents a
// communication line to a Service which can be used to call that Service's
// methods.  The Service may be running on another machine.  Normally, you
// should not call an RpcChannel directly, but instead construct a stub Service
// XXX
// wrapping it.  Example:
// FIXME: update here
//   RpcChannel* channel = new MyRpcChannel("remotehost.example.com:1234");
//   MyService* service = new MyService::Stub(channel);
//   service->MyMethod(request, &response, callback);
class RpcChannel : public ::google::protobuf::RpcChannel
{
public:
	RpcChannel();
	explicit RpcChannel(const RpcSendTunnelPtr& tunnel);
	~RpcChannel();

	void set_sendtunnel(const RpcSendTunnelPtr& tunnel)
	{
		tunnel_ = tunnel;
	}

	const RpcSendTunnelPtr get_sendtunnel() const { return tunnel_; }

	void set_services(const std::map<std::string, ::google::protobuf::Service*>* services)
	{
		services_ = services;
	}

	void set_caller_info(int64_t id, int32_t type)
	{
		caller_id_   = id;
		caller_type_ = type;
	}

	void set_timer(TimedTaskManager* timer)
	{
		timer_ = timer;
	}

	inline int64_t get_caller_id() const { return caller_id_; }
	
	// Call the given method of the remote service.  The signature of this
	// procedure looks the same as Service::CallMethod(), but the requirements
	// are less strict in one important way:  the request and response objects
	// need not be of any specific class as long as their descriptors are
	// method->input_type() and method->output_type().
	void CallMethod(const ::google::protobuf::MethodDescriptor* method,
			::google::protobuf::RpcController* controller,
			const ::google::protobuf::Message* request,
			::google::protobuf::Message* response,
			::google::protobuf::Closure* done);

	void OnRpcMessage(int32_t sendback_sid,
			          const RpcMessage& message);
	
	void     TimeoutHeartbeat(Timestamp cur_time);
	int32_t  GetNextTimeoutSecs(Timestamp cur_time, int32_t default_interval);


private:
	void CallServiceMethod(int32_t sendback_sid, const RpcMessage& message);

	struct BackToClient
	{
		int64_t id;
		int64_t caller_id;
		int32_t caller_type;
		int32_t sendback_sid;
	};

	void DoneCallback(::google::protobuf::Message* response, BackToClient data);

	struct OutstandingCall
	{
		::google::protobuf::RpcController* controller;
		::google::protobuf::Message* response;
		::google::protobuf::Closure* done;
	};

	typedef std::pair<Timestamp, int64_t> Entry;
	std::vector<Entry> get_expired(Timestamp now);
	void HandleTimeout(OutstandingCall& out);
	void RpcTimeoutCallback(const TimedTask* p);
	void CancelTimeout(int64_t id);

	
	RpcSendTunnelPtr tunnel_;
	AtomicInt64 id_;

	MutexLock mutex_;
	typedef std::map<int64_t, OutstandingCall> OutstandindMap;
	OutstandindMap outstandings_;

	typedef std::map<std::string, ::google::protobuf::Service*> ServiceMap;
	const ServiceMap* services_;

	int64_t    caller_id_;   // for RpcClient
	int32_t    caller_type_; // for RpcClient

	typedef std::set<Entry> TimeoutSet;
	TimeoutSet         handle_timeout_set_;

	TimedTaskManager*  timer_;

	// store <id, TimedTaskId>, if timer_ valid
	// store <id, Timestamp>, if handle_timeout_set_ valid
	typedef std::map<int64_t, int64_t> CancelTimeoutMap;
	CancelTimeoutMap   cancel_timeout_map_;
};
 
} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_RPC_CHANNEL_H_
