#include "player_ctrl.h"

#include "gs/scene/slice.h"

#include "player.h"


namespace gamed {

PlayerController::PlayerController(Player& player)
	: player_(player),
	  cmd_disp_(BIND_MEM_CB(&PlayerController::PacketDefaultHandler, this)),
	  debug_cmd_disp_(BIND_MEM_CB(&PlayerController::PacketDefaultHandler, this))
{
	StartupNormalCmdDispRegister();
	StartupDebugCmdDispRegister();
}

PlayerController::~PlayerController()
{
}

int PlayerController::CommandHandler(ProtoPacket* packet)
{
	cmd_disp_.OnProtoPacket(*packet);
	return 0;
}

int PlayerController::UnLockInventoryHandler(ProtoPacket* packet)
{
	return 0;
}

int PlayerController::InvalidCommandHandler(ProtoPacket* packet)
{
	return 0;
}

int PlayerController::DebugCommandHandler(ProtoPacket* packet)
{
	debug_cmd_disp_.OnProtoPacket(*packet);
	return 0;
}

int PlayerController::GMCommandHandler(ProtoPacket* packet)
{
	return 0;
}

} // namespace gamed
