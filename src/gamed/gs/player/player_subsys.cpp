#include "player_subsys.h"

#include "subsys_if.h"
#include "player_ctrl.h"
#include "player.h"
#include "player_sender.h"


namespace gamed {

using namespace shared::net;

PlayerSubSystem::PlayerSubSystem(SubSysType type, Player& player)
	: type_(type),
	  player_(player),
	  subsys_if_(*player.subsys_if())
{
}

PlayerSubSystem::~PlayerSubSystem()
{
}

void PlayerSubSystem::Initialize()
{
	RegisterCmdHandler();
	RegisterMsgHandler();
	OnInit();
}

void PlayerSubSystem::SendCmd(const PacketRef packet) const
{
	player_.sender()->SendCmd(packet);
}

void PlayerSubSystem::BroadCastCmd(const PacketRef packet) const
{
	player_.sender()->BroadCastCmd(packet);
}

void PlayerSubSystem::SendError(int err_no) const
{
	player_.sender()->ErrorMessage(err_no);
}

} // namespace gamed
