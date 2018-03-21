#ifndef SHARED_NET_PACKET_CODEC_FOR_LINK_H_
#define SHARED_NET_PACKET_CODEC_FOR_LINK_H_

#include "shared/net/packet/codec_packet.h"

namespace shared {
namespace net {

class ProtoPacketCodecForLink : public ProtoPacketCodec
{
public:
	virtual PacketPtr Parse(const char* buf, int len, ErrorCode* errorCode);
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_CODEC_FOR_LINK_H_
