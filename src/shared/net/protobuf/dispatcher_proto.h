#ifndef SHARED_NET_PROTOBUF_DISPATCHER_H_
#define SHARED_NET_PROTOBUF_DISPATCHER_H_

#include <assert.h>
#include <stdio.h>
#include <google/protobuf/message.h>
#include <map>

#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"

namespace shared {

typedef google::protobuf::Message& MessageRef;

class ProtobufCallback : noncopyable
{
public:
	ProtobufCallback(int status)
		: status_(status)
	{ }
	virtual ~ProtobufCallback() {}
	virtual void OnMessage(const MessageRef message) const = 0;

	int get_status() const { return status_; }

protected:
	int status_;
};

template <typename T>
class ProtobufCallbackT : public ProtobufCallback
{
public:
	typedef shared::bind::Callback<void (const T&)> ProtobufMessageTCallback;

	ProtobufCallbackT(const ProtobufMessageTCallback& callback, int status)
		: ProtobufCallback(status), callback_(callback)
	{ }

	virtual void OnMessage(const MessageRef message) const
	{
		T* concrete = dynamic_cast<T*>(&message);
		assert(concrete != NULL);
		callback_(*concrete);
	}

private:
	ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
public:
	typedef shared::bind::Callback<void (const MessageRef)> ProtobufMsgDefaultCallback;

	explicit ProtobufDispatcher(const ProtobufMsgDefaultCallback& defaultcb)
		: default_callback_(defaultcb)
	{ }

	~ProtobufDispatcher()
	{
		CallbackMap::iterator it = callbacks_.begin();
		for (; it != callbacks_.end(); ++it)
		{
			delete it->second;
		}
	}

	void OnProtobufMessage(const MessageRef message, int status = 0xFFFFFFFF) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message.GetDescriptor());
		if (it != callbacks_.end() && (status & it->second->get_status())) // check status to dispatch
		{
			it->second->OnMessage(message);
		}
		else
		{
			default_callback_(message);
		}
	}

	template<typename T>
	void Register(const typename ProtobufCallbackT<T>::ProtobufMessageTCallback& callback, 
			      int status_mask_can_dispatch = 0xFFFFFFFF)
	{
		CallbackMap::const_iterator it = callbacks_.find(T::descriptor());
		if (it != callbacks_.end())
		{
			fprintf(stderr, "Error: Multiple registration - by same proto\n");
			assert(false);
			return;
		}
		ProtobufCallback* pcallback = new ProtobufCallbackT<T>(callback, status_mask_can_dispatch);
		callbacks_[T::descriptor()] = pcallback;
	}

private:
	typedef std::map<const google::protobuf::Descriptor*, ProtobufCallback*> CallbackMap;

	CallbackMap callbacks_;
	ProtobufMsgDefaultCallback default_callback_;
};

} // namespace shared

#endif // SHARED_NET_PROTOBUF_DISPATCHER_H_
