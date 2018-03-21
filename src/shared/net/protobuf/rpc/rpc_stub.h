#ifndef SHARED_NET_PROTOBUF_RPC_STUB_H_
#define SHARED_NET_PROTOBUF_RPC_STUB_H_

#include <google/protobuf/service.h>

#include "shared/net/protobuf/rpc/rpc_callback.h"


namespace shared {
namespace net {

///
/// use in rpc client side
///
class ServiceStub : public noncopyable
{
public:
	ServiceStub() 
		: is_done_(false)
	{ }

	virtual ~ServiceStub()
	{
		SAFE_DELETE(callback_);
	}
	
	virtual void registerCall(int32_t sendto_sid, 
			                  RpcCallback* callback, 
							  int32_t timeout_secs) = 0;

	inline bool is_done() const { return is_done_; }
	inline void set_callback(RpcCallback* callback) { callback_ = callback; }
	inline void set_is_done() { is_done_ = true; }

protected:
	bool         is_done_;
	RpcCallback* callback_;
};

class RpcStubController: public ::google::protobuf::RpcController
{
public:
	RpcStubController()
		: err_text_(),
		  is_canceled_(false)
	{ }

	virtual ~RpcStubController()
	{ }

	virtual void Reset()
	{ }

	virtual bool Failed() const
	{
		return !err_text_.empty();
	}

	virtual std::string ErrorText() const
	{
		return err_text_;
	}

	virtual void StartCancel()
	{
		is_canceled_ = true;
	}

	virtual void SetFailed(const std::string& reason)
	{
		err_text_ = reason;
	}

	virtual bool IsCanceled() const
	{
		return is_canceled_;
	}

	virtual void NotifyOnCancel(::google::protobuf::Closure* callback)
	{ }

	void    set_sendto_sid(int32_t sid) { sendto_sid_ = sid; }
	int32_t sendto_sid() { return sendto_sid_; }

	void    set_timeout_secs(int32_t secs) { timeout_secs_ = secs; }
	int32_t timeout_secs() { return timeout_secs_; }


private:
	GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RpcStubController);

	std::string err_text_;
	bool        is_canceled_;
	int32_t     sendto_sid_;
	int32_t     timeout_secs_;
};


} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_RPC_STUB_H_
