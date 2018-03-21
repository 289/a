#ifndef SHARED_NET_PACKET_DISPATCHER_H_
#define SHARED_NET_PACKET_DISPATCHER_H_

#include <assert.h>
#include <stdio.h>
#include <map>

#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"

#include "shared/net/packet/proto_packet.h"

namespace shared {
namespace net {

typedef shared::net::ProtoPacket& PacketRef;

class PacketCallback : noncopyable
{
public:
	PacketCallback(int status)
		: status_(status)
	{}
	virtual ~PacketCallback() {}
	virtual void OnMessage(const PacketRef message) const = 0;

	int get_status() const { return status_; }

protected:
	int status_;
};

template <typename T>
class PacketCallbackT : public PacketCallback
{
public:
	typedef shared::bind::Callback<void (const T&)> ProtoPacketTCallback;

	PacketCallbackT(const ProtoPacketTCallback& callback, int status)
		: PacketCallback(status), callback_(callback)
	{ }

	virtual void OnMessage(const PacketRef message) const
	{
		T* concrete = dynamic_cast<T*>(&message);
		assert(concrete != NULL);
		callback_(*concrete);
	}

private:
	ProtoPacketTCallback callback_;
};

class ProtoPacketDispatcher
{
public:
	typedef shared::bind::Callback<void (const PacketRef)> ProtoPacketDefaultCallback;

	explicit ProtoPacketDispatcher(const ProtoPacketDefaultCallback& defaultcb)
		: default_callback_(defaultcb)
	{ }

	~ProtoPacketDispatcher()
	{
		CallbackMap::iterator it = callbacks_.begin();
		for (; it != callbacks_.end(); ++it)
		{
			delete it->second;
		}
	}

	void OnProtoPacket(const PacketRef message, int status = 0xFFFFFFFF) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message.GetType());
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
	void Register(const typename PacketCallbackT<T>::ProtoPacketTCallback& callback, int status_mask_can_dispatch = 0xFFFFFFFF)
	{
		CallbackMap::const_iterator it = callbacks_.find(T::TypeNumber());
		if (it != callbacks_.end())
		{
			fprintf(stderr, "Error: Multiple registration - by same proto\n");
			assert(false);
			return;
		}

		PacketCallback* pcallback = new PacketCallbackT<T>(callback, status_mask_can_dispatch);
		callbacks_[T::TypeNumber()] = pcallback;
	}

private:
	typedef std::map<const ProtoPacket::Type, PacketCallback*> CallbackMap;

	CallbackMap callbacks_;
	ProtoPacketDefaultCallback default_callback_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_DISPATCHER_H_
