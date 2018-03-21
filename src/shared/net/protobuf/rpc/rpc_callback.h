#ifndef SHARED_NET_PROTOBUF_RPC_CALLBACK_H_
#define SHARED_NET_PROTOBUF_RPC_CALLBACK_H_

#include "shared/base/callback_bind.h"
#include "shared/net/protobuf/rpc/rpc_service.h"


namespace shared {
namespace net {

typedef int RPCErrorCode;

namespace rpc 
{
	enum ErrorCode
	{
		kNoError = 0,
		kTimeout,
		kErrorUnknown,
	};
} // namespace rpc

class ServiceStub;

class RpcCallback : noncopyable
{
public:
	RpcCallback()
	{ }
	virtual ~RpcCallback() {}
	virtual void OnCallback(const ::google::protobuf::Message& response,
			                RPCErrorCode err_code) const = 0;
};

template <typename T>
class RpcCallbackT : public RpcCallback
{
public:
	typedef shared::bind::Callback<void (const T&, 
			                             RPCErrorCode,
										 const ServiceStub*)> RpcMessageTCallback;

	RpcCallbackT(const RpcMessageTCallback& callback, ServiceStub* stub)
		: callback_(callback),
		  pstub_(stub)
	{ }

	virtual void OnCallback(const ::google::protobuf::Message& response,
			                RPCErrorCode err_code) const
	{
		const T* concrete = dynamic_cast<const T*>(&response);
		assert(concrete != NULL);
		callback_(*concrete, err_code, pstub_);
	}

private:
	RpcMessageTCallback callback_;
	ServiceStub* pstub_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_RPC_CALLBACK_H_
