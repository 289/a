#ifndef GAMED_GS_OBJ_NPC_SENDER_H_
#define GAMED_GS_OBJ_NPC_SENDER_H_

#include <stdint.h>
#include <map>

#include "shared/base/types.h"
#include "shared/base/noncopyable.h"

#include "gs/netmsg/send_to_link.h"
#include "gs/global/math_types.h"


namespace shared 
{
	namespace net 
	{
		class ProtoPacket;
	} // namespace net
} // namespace shared


namespace gamed {

class Npc;

///
/// NpcSender
///
class NpcSender : shared::noncopyable
{
	typedef shared::net::ProtoPacket& PacketRef;
public:
	NpcSender(Npc& npc);
	virtual ~NpcSender();

	void    PlayerEnterView(int64_t role_id, int32_t link_id, int32_t sid_in_link);
	void    PlayerLeaveView(RoleID id);

    // movement
	void    Move(const A2DVECTOR& dest, uint16_t use_time, uint16_t speed, uint8_t move_mode);
	void    MoveStop(const A2DVECTOR& pos, uint16_t speed, uint8_t dir, uint8_t move_mode);
    void    NotifyObjectPos(const A2DVECTOR& pos, uint8_t dir);


public:
	void    AutoBroadcastCSMsg(PacketRef packet);
	void    SendToPlayer(RoleID roleid, int32_t linkid, int32_t client_sid, PacketRef packet);


private:
	Npc&    npc_;

	typedef std::vector<NetToLink::MulticastPlayerInfo> PlayerInfoVec;
	typedef std::map<int32_t, PlayerInfoVec> PlayersInFieldOfView;
	PlayersInFieldOfView    players_in_view_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_NPC_SENDER_H_
