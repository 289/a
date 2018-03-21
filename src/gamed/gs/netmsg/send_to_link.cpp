#include "send_to_link.h"

#include "shared/logsys/logging.h"
#include "shared/net/packet/codec_packet.h"

#include "gs/netio/netio_if.h"

// proto
#include "common/protocol/gen/G2L/s2cgamedatasend.pb.h"
#include "common/protocol/gen/G2L/s2cmulticast.pb.h"
#include "common/protocol/gen/G2L/player_enterworld_re.pb.h"
#include "common/protocol/gen/G2L/redirect_player_gs.pb.h"


namespace gamed {

using namespace common::protocol;

void NetToLink::SendProtocol(int32_t linkid, const ProtobufCodec::MessageRef msg_ref)
{
	NetIO::SendToLink(linkid, msg_ref);
}

void NetToLink::SendS2CGameData(int32_t linkid, RoleID roleid, int32_t client_sid, PacketRef packet)
{
	G2L::S2CGamedataSend game_data;
	game_data.set_cmd_type_no(packet.GetType());
	game_data.set_roleid(roleid);
	game_data.set_link_id(linkid);
	game_data.set_client_sid_in_link(client_sid);

	// GetSize() == 0 判断是否已经做了Marshal
	if (packet.GetSize() == 0 && !ProtoPacketCodec::MarshalPacket(packet))
	{
		LOG_ERROR << "SendS2CGameData() MarshalPacket error packet_type:" << packet.GetType();
		return;
	}
	game_data.set_content(packet.GetContent(), packet.GetSize());

	NetIO::SendToLink(linkid, game_data);
}

void NetToLink::SendS2CMulticast(int32_t linkid, const std::vector<MulticastPlayerInfo>& info_list, PacketRef packet)
{
	if (info_list.size() == 0) return;

	G2L::S2CMulticast multi_game_data;
	multi_game_data.set_cmd_type_no(packet.GetType());
	multi_game_data.set_link_id(linkid);

	// GetSize() == 0 判断是否已经做了Marshal，因为本函数常在循环中调用，防止Marshal多次
	if (packet.GetSize() == 0 && !ProtoPacketCodec::MarshalPacket(packet))
	{
		LOG_ERROR << "SendS2CMulticast() MarshalPacket error packet_type:" << packet.GetType();
		return;
	}
	multi_game_data.set_content(packet.GetContent(), packet.GetSize());

	for (size_t i = 0; i < info_list.size(); ++i)
	{
		G2L::S2CMulticast::PlayerInfo* tmpinfo = multi_game_data.add_player_list();
		tmpinfo->set_roleid(info_list[i].role_id);
		tmpinfo->set_client_sid_in_link(info_list[i].sid_in_link);
	}

	NetIO::SendToLink(linkid, multi_game_data);
}

void NetToLink::RedirectPlayerGS(int32_t linkid, RoleID roleid)
{
	G2L::RedirectPlayerGS proto;
	proto.set_roleid(roleid);
	NetIO::SendToLink(linkid, proto);
}

void NetToLink::PlayerEnterWorldReply(int32_t linkid, RoleID roleid, int32_t client_sid_in_link)
{
	G2L::PlayerEnterWorld_Re proto;
	proto.set_roleid(roleid);
	proto.set_client_sid_in_link(client_sid_in_link);
	NetIO::SendToLink(linkid, proto);
}

} // namespace gamed
