#ifndef GAMED_GS_NETMSG_SEND_TO_LINK_H_
#define GAMED_GS_NETMSG_SEND_TO_LINK_H_

#include <stdint.h>
#include <vector>

#include "shared/base/types.h"
#include "shared/net/protobuf/codec_proto.h"


namespace shared {
namespace net {

class ProtoPacket;

} // namespace net
} // namespace shared


namespace gamed {

class NetToLink
{
	typedef shared::net::ProtoPacket& PacketRef;
public:
	struct MulticastPlayerInfo
	{
		int64_t role_id;
		int32_t sid_in_link;
	};

    // ---- thread safe ----
	static void SendProtocol(int32_t linkid, const shared::net::ProtobufCodec::MessageRef msg_ref);
	static void SendS2CGameData(int32_t linkid, RoleID roleid, int32_t client_sid, PacketRef packet);
	static void SendS2CMulticast(int32_t linkid, const std::vector<MulticastPlayerInfo>& info_list, PacketRef packet);
	static void RedirectPlayerGS(int32_t linkid, RoleID roleid);
	static void PlayerEnterWorldReply(int32_t linkid, RoleID roleid, int32_t client_sid_in_link);
};

} // namespace NetToLink

#endif // GAMED_GS_NETMSG_SEND_TO_LINK_H_
