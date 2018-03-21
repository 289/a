#include "matter_sender.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gs/global/game_def.h"

#include "matter.h"


namespace gamed {

MatterSender::MatterSender(Matter& matter)
	: matter_(matter)
{
}

MatterSender::~MatterSender()
{
}

void MatterSender::AutoBroadcastCSMsg(PacketRef packet)
{
	PlayersInFieldOfView::const_iterator it = players_in_view_.begin();
	for (; it != players_in_view_.end(); ++it)
	{
		NetToLink::SendS2CMulticast(it->first, it->second, packet);
	}
}

void MatterSender::SendToPlayer(RoleID roleid, int32_t linkid, int32_t client_sid, PacketRef packet)
{
	NetToLink::SendS2CGameData(linkid, roleid, client_sid, packet);
}

void MatterSender::PlayerEnterView(int64_t role_id, int32_t link_id, int32_t sid_in_link)
{
	NetToLink::MulticastPlayerInfo tmpplayerinfo;
	tmpplayerinfo.role_id     = role_id;
	tmpplayerinfo.sid_in_link = sid_in_link;
	players_in_view_[link_id].push_back(tmpplayerinfo);

	G2C::ObjectEnterView packet;
	packet.obj_info.obj_id = static_cast<int32_t>(matter_.object_id()); 
	packet.obj_info.tid    = matter_.templ_id();
	packet.obj_info.eid    = matter_.elem_id();
	packet.obj_info.dir    = matter_.dir();
	packet.obj_info.pos    = matter_.pos();
	SendToPlayer(role_id, link_id, sid_in_link, packet);
}

void MatterSender::PlayerLeaveView(RoleID id)
{
	int count = 0;
	PlayersInFieldOfView::iterator it_map = players_in_view_.begin();
	while (it_map != players_in_view_.end())
	{
		PlayerInfoVec& players_vec = it_map->second;
		PlayerInfoVec::iterator it_vec = players_vec.begin();
		while (it_vec != players_vec.end())
		{
			if ((*it_vec).role_id == id) {
				it_vec = players_vec.erase(it_vec);
				++count;
			}
			else {
				++it_vec;
			}
		}
		++it_map;
	}

	// 有可能EnterView的时候，没有QueryPlayer到玩家，所以不能ASSERT
	//ASSERT(count == 1);
	if (count != 1)
	{   
		LOG_WARN << "NpcSender::PlayerLeaveView count:" << count;
	}
}

void MatterSender::NotifyObjectPos(const A2DVECTOR& pos, uint8_t dir)
{
    G2C::NotifyObjectPos packet;
    packet.obj_id = matter_.object_id();
    packet.pos    = pos;
    packet.dir    = dir;

    AutoBroadcastCSMsg(packet);
}

} // namespace gamed
