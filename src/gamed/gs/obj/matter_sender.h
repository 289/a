#ifndef GAMED_GS_OBJ_MATTER_SENDER_H_
#define GAMED_GS_OBJ_MATTER_SENDER_H_

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

class Matter;

///
/// MatterSender
///
class MatterSender : shared::noncopyable
{
	typedef shared::net::ProtoPacket& PacketRef;
public:
	MatterSender(Matter& matter);
	virtual ~MatterSender();

	void    PlayerEnterView(int64_t role_id, int32_t link_id, int32_t sid_in_link);
	void    PlayerLeaveView(RoleID id);

    // movement
    void    NotifyObjectPos(const A2DVECTOR& pos, uint8_t dir);


protected:
	void    AutoBroadcastCSMsg(PacketRef packet);
	void	SendToPlayer(RoleID roleid, int32_t linkid, int32_t client_sid, PacketRef packet);


private:
	Matter& matter_;

	typedef std::vector<NetToLink::MulticastPlayerInfo> PlayerInfoVec;
	typedef std::map<int32_t, PlayerInfoVec> PlayersInFieldOfView;
	PlayersInFieldOfView    players_in_view_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_MATTER_SENDER_H_
