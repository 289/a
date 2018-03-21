#ifndef GAMED_GS_NETMSG_DISPATCHER_PROTO_H_
#define GAMED_GS_NETMSG_DISPATCHER_PROTO_H_

#include <assert.h>
#include <stdio.h>
#include <google/protobuf/message.h>
#include <map>

#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"

#include "gamed/gs/netio/queue_msg_type.h"


namespace gamed {

using namespace shared;

typedef google::protobuf::Message& MessageRef;
typedef network::QueuedRecvMsg& WrapRef;

class QueuedMsgCallback : noncopyable
{
public:
	QueuedMsgCallback(int status)
		: status_(status)
	{ }
	virtual ~QueuedMsgCallback() {}
	virtual void OnMessage(const MessageRef message, const WrapRef wrap) const = 0;

	int get_status() const { return status_; }

protected:
	int status_;
};

template <typename T>
class QueuedMsgCallbackT : public QueuedMsgCallback
{
public:
	typedef shared::bind::Callback<void (const T&, const WrapRef)> QueuedMessageTCallback;

	QueuedMsgCallbackT(const QueuedMessageTCallback& callback, int status)
		: QueuedMsgCallback(status), callback_(callback)
	{ }

	virtual void OnMessage(const MessageRef message, const WrapRef wrap) const
	{
		T* concrete = dynamic_cast<T*>(&message);
		assert(concrete != NULL);
		callback_(*concrete, wrap);
	}

private:
	QueuedMessageTCallback callback_;
};

class QueuedMsgDispatcher
{
public:
	typedef shared::bind::Callback<void (const MessageRef, const WrapRef)> QueuedMsgDefaultCallback;

	explicit QueuedMsgDispatcher(const QueuedMsgDefaultCallback& defaultcb)
		: default_callback_(defaultcb)
	{ }

	~QueuedMsgDispatcher()
	{
		CallbackMap::iterator it = callbacks_.begin();
		for (; it != callbacks_.end(); ++it)
		{
			delete it->second;
		}
	}

	void OnQueuedMessage(const MessageRef message, const WrapRef wrap, int status = 0xFFFFFFFF) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message.GetDescriptor());
		if (it != callbacks_.end() && (status & it->second->get_status())) // check status to dispatch
		{
			it->second->OnMessage(message, wrap);
		}
		else
		{
			default_callback_(message, wrap);
		}
	}

	template<typename T>
	void Register(const typename QueuedMsgCallbackT<T>::QueuedMessageTCallback& callback, 
			      int status_mask_can_dispatch = 0xFFFFFFFF)
	{
		CallbackMap::const_iterator it = callbacks_.find(T::descriptor());
		if (it != callbacks_.end())
		{
			fprintf(stderr, "Error: Multiple registration - by same queuedmsg\n");
			assert(false);
			return;
		}
		QueuedMsgCallback* pcallback = new QueuedMsgCallbackT<T>(callback, status_mask_can_dispatch);
		callbacks_[T::descriptor()] = pcallback;
	}

private:
	typedef std::map<const google::protobuf::Descriptor*, QueuedMsgCallback*> CallbackMap;

	CallbackMap callbacks_;
	QueuedMsgDefaultCallback default_callback_;
};

} // namespace gamed

#endif // GAMED_GS_NETMSG_DISPATCHER_PROTO_H_
