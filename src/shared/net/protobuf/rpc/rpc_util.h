#ifndef SHARED_NET_PROTOBUF_RPC_UTIL_H_
#define SHARED_NET_PROTOBUF_RPC_UTIL_H_

#include "shared/net/protobuf/rpc/rpc_service.h"

///
/// rpc service stub
///
#define DEFINE_RPC_STUB(stub_class, Package, RpcPrefix, Method) \
	DEFINE_RPC_STUB_TEMPLATE(stub_class, Package, RpcPrefix##Service, RpcPrefix##Request, RpcPrefix##Response, Method)

/// template
#define DEFINE_RPC_STUB_TEMPLATE(stub_class, Package, Service, Request, Response, Method) \
	public: \
		stub_class(shared::net::RpcChannel* channel) \
			: stub_(channel), \
			  response_(new Package::Response()) \
		{ } \
\
		Package::Request& request() { return request_; } \
		Package::Response* response() { return response_; } \
		shared::net::RpcStubController& controller() { return controller_; } \
		const Package::Request* get_request() const { return &request_; } \
\
		virtual void registerCall(int32_t sendto_sid, RpcCallback* callback, int32_t timeout_secs) \
		{ \
			assert(callback); \
			set_callback(callback); \
			controller_.set_sendto_sid(sendto_sid); \
			controller_.set_timeout_secs(timeout_secs); \
			stub_.Method(&controller_, &request_, response_,  \
					NewCallback(this, &stub_class::solved, response_, &controller_)); \
		} \
\
		void solved(Package::Response* resp, shared::net::RpcStubController* controller) \
		{ \
			if (controller->ErrorText() == std::string(shared::net::rpc::kTimeoutErrorText)) \
			{ \
				callback_->OnCallback(*(static_cast<google::protobuf::Message*>(resp)), shared::net::rpc::kTimeout); \
			} \
			else if (!controller->Failed()) \
			{ \
				callback_->OnCallback(*(static_cast<google::protobuf::Message*>(resp)), shared::net::rpc::kNoError); \
			} \
			else \
			{ \
				callback_->OnCallback(*(static_cast<google::protobuf::Message*>(resp)), shared::net::rpc::kErrorUnknown); \
			} \
			is_done_ = true; \
		} \
\
	private: \
		Package::Service::Stub stub_; \
		Package::Request       request_; \
		Package::Response*     response_; \
		shared::net::RpcStubController controller_;


#endif // SHARED_NET_PROTOBUF_RPC_UTIL_H_
