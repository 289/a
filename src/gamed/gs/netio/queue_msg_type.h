#ifndef GAMED_GS_NETWORK_QUEUE_MSG_TYPE_H_
#define GAMED_GS_NETWORK_QUEUE_MSG_TYPE_H_

#include "shared/base/copyable.h"
#include "shared/base/blocking_queue.h"
#include "shared/base/rwlock_queue.h"
#include "shared/net/protobuf/codec_proto.h"


namespace gamed {
namespace network {

//typedef shared::net::ProtobufCodec::MessagePtr QueuedRecvMsg;
//typedef shared::net::ProtobufCodec::MessageRef SendMessageRef;

class QueuedRecvMsg : public shared::copyable
{
public:
	QueuedRecvMsg()
		: sendback_sid(-1),
		  msg_ptr(NULL)
	{ }

	QueuedRecvMsg(int32_t sendbackSid, 
			      shared::net::ProtobufCodec::MessagePtr msgPtr)
		: sendback_sid(sendbackSid),
		  msg_ptr(msgPtr)
	{ }

	int32_t sendback_sid;
	shared::net::ProtobufCodec::MessagePtr msg_ptr;
};

typedef shared::RWLockQueue<QueuedRecvMsg> LinkRecvQueue;
typedef shared::BlockingQueue<QueuedRecvMsg> MasterRecvQueue;

} // namespace network
} // namespace gamed

#endif //GAMED_GS_NETWORK_QUEUE_MSG_TYPE_H_
