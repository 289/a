#include "npc_sender.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gs/template/map_data/mapdata_manager.h"

#include "npc.h"


namespace gamed {

namespace {
	
	inline bool can_see_me(int32_t elem_id)
	{
		const mapDataSvr::BaseMapData* pbase = s_pMapData->QueryBaseMapDataTempl(elem_id);
		if (pbase != NULL)
		{
			if (pbase->GetType() == mapDataSvr::MAPDATA_TYPE_AREA_NPC)
				return false;
		}

		return true;
	}

} // Anonymous


NpcSender::NpcSender(Npc& npc)
	: npc_(npc)
{
}

NpcSender::~NpcSender()
{
}

void NpcSender::AutoBroadcastCSMsg(PacketRef packet)
{
	PlayersInFieldOfView::const_iterator it = players_in_view_.begin();
	for (; it != players_in_view_.end(); ++it)
	{
		NetToLink::SendS2CMulticast(it->first, it->second, packet);
	}
}

void NpcSender::SendToPlayer(RoleID roleid, int32_t linkid, int32_t client_sid, PacketRef packet)
{
	NetToLink::SendS2CGameData(linkid, roleid, client_sid, packet);
}

void NpcSender::PlayerEnterView(int64_t role_id, int32_t link_id, int32_t sid_in_link)
{
	if (!can_see_me(npc_.elem_id()))
	{
		return;
	}

	NetToLink::MulticastPlayerInfo tmpplayerinfo;
	tmpplayerinfo.role_id     = role_id;
	tmpplayerinfo.sid_in_link = sid_in_link;
	players_in_view_[link_id].push_back(tmpplayerinfo);

	G2C::ObjectEnterView packet;
	packet.obj_info.obj_id   = static_cast<int32_t>(npc_.object_id()); 
	packet.obj_info.tid      = npc_.templ_id();
	packet.obj_info.eid      = npc_.elem_id();
	packet.obj_info.dir      = npc_.dir();
	packet.obj_info.pos      = npc_.pos();
	SendToPlayer(role_id, link_id, sid_in_link, packet);
}

void NpcSender::PlayerLeaveView(RoleID id)
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
	if (count != 1 && can_see_me(npc_.elem_id()))
	{   
		LOG_WARN << "NpcSender::PlayerLeaveView count:" << count;
	}
}

void NpcSender::Move(const A2DVECTOR& dest, uint16_t use_time, uint16_t speed, uint8_t move_mode)
{
	G2C::ObjectMove packet;
	packet.obj_id    = npc_.object_id();
	packet.dest      = dest;
	packet.use_time  = use_time;
	packet.speed     = speed;
	packet.move_mode = move_mode;

	AutoBroadcastCSMsg(packet);
}

void NpcSender::MoveStop(const A2DVECTOR& pos, uint16_t speed, uint8_t dir, uint8_t move_mode)
{
	G2C::ObjectStopMove packet;
	packet.obj_id    = npc_.object_id();
	packet.pos       = pos;
	packet.speed     = speed;
	packet.dir       = dir;
	packet.move_mode = move_mode;

	AutoBroadcastCSMsg(packet);
}

void NpcSender::NotifyObjectPos(const A2DVECTOR& pos, uint8_t dir)
{
    G2C::NotifyObjectPos packet;
    packet.obj_id = npc_.object_id();
    packet.pos    = pos;
    packet.dir    = dir;

    AutoBroadcastCSMsg(packet);
}

} // namespace gamed
