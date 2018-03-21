#include "combat_player.h"
#include "combat.h"

namespace gamed
{

void CombatPlayer::GetTeamMate(std::vector<ObjInterface*>& players) const
{
	Combat::GetPlayer(team_, players);
}

void CombatPlayer::GetEnemy(std::vector<ObjInterface*>& players) const
{
	Combat::GetPlayer((team_ + 1) % 2, players);
}

} // namespace gamed
