#ifndef GAMED_GS_PLAYER_CTRL_H_
#define GAMED_GS_PLAYER_CTRL_H_

#include "shared/net/packet/proto_packet.h"
#include "shared/net/packet/dispatcher_packet.h"

#include "gamed/client_proto/C2G_proto.h"


namespace gamed {

using namespace shared::net;

class Slice;
class Player;
class PlayerController
{
public:
	PlayerController(Player& player);
	~PlayerController();

	// cmd handler
	int    CommandHandler(ProtoPacket* packet);
	int    UnLockInventoryHandler(ProtoPacket* packet);
	int    InvalidCommandHandler(ProtoPacket* packet);
	int    DebugCommandHandler(ProtoPacket* packet);
	int    GMCommandHandler(ProtoPacket* packet);
	
	inline ProtoPacketDispatcher& cmd_disp();
	inline ProtoPacketDispatcher& debug_cmd_disp();


protected:
	void   StartupNormalCmdDispRegister();
	void   StartupDebugCmdDispRegister();
	void   PacketDefaultHandler(const PacketRef);

	//
	// CMD处理函数
	//
	void   StartMove(const C2G::StartMove&);
	void   MoveContinue(const C2G::MoveContinue&);
	void   StopMove(const C2G::StopMove&);
	void   GetAllData(const C2G::GetAllData&);
	void   SelectResurrectPos(const C2G::SelectResurrectPos&);
	void   JoinTeam(const C2G::JoinTeam&);
	void   JoinTeamRes(const C2G::JoinTeamRes&);
	void   OpenCatVision(const C2G::OpenCatVision&);
	void   CloseCatVision(const C2G::CloseCatVision&);
	void   GetElsePlayerExtProp(const C2G::GetElsePlayerExtProp&);
	void   GetElsePlayerEquipCRC(const C2G::GetElsePlayerEquipCRC&);
	void   GetElsePlayerEquipment(const C2G::GetElsePlayerEquipment&);
	void   GetStaticRoleInfo(const C2G::GetStaticRoleInfo&);
	void   QueryNpcZoneInfo(const C2G::QueryNpcZoneInfo&);
	void   QueryServerTime(const C2G::QueryServerTime&);
	
	///
	/// DEBUG_CMD处理函数
	///
	void   DebugCommonCmd(const C2G::DebugCommonCmd&);
	void   DebugChangePlayerPos(const C2G::DebugChangePlayerPos&);
    void   DebugChangeFirstName(const C2G::DebugChangeFirstName&);


private:
	bool   HandleMoveInCombat(uint16_t seq_from_cli, uint16_t speed, uint8_t mode);


private:
	Player&    player_;

	ProtoPacketDispatcher cmd_disp_;
	ProtoPacketDispatcher debug_cmd_disp_;
};

///
/// inline func
///
inline ProtoPacketDispatcher& PlayerController::cmd_disp()
{
	return cmd_disp_;
}

inline ProtoPacketDispatcher& PlayerController::debug_cmd_disp()
{
	return debug_cmd_disp_;
}

} // namespace gamed

#endif // GAMED_GS_PLAYER_CTRL_H_
