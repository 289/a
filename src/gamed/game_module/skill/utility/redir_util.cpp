#include "redir_util.h"
#include "redirect.h"
#include "obj_interface.h"
#include "player_util.h"
#include "util.h"

namespace skill
{
	
// 重定向只对非死亡单位生效
int8_t RedirUtil::GetTarget(Player* caster, Player* trigger, const Redirect& redir, PlayerVec& target)
{
	switch (redir.type)
	{
	case REDIR_NO:
		break;
	case REDIR_CASTER:
		target.push_back(caster);
		break;
	case REDIR_TRIGGER:
		target.push_back(trigger);
		break;
	case REDIR_ALL_MATE:
	case REDIR_ALL_ENEMY:
		AllAlive(caster, redir, target);
		break;
	case REDIR_RANDOM_MATE:
	case REDIR_RANDOM_ENEMY:
		RandTarget(caster, redir, target);
		break;
	default:
		break;
	}
	return target.size();
}

void RedirUtil::AllAlive(Player* caster, const Redirect& redir, PlayerVec& target)
{
	TargetTeam team = redir.type == REDIR_ALL_MATE ? TEAM_MATE : TEAM_ENEMY;
	PlayerVec all;
	PlayerUtil::GetMembers(caster, team, all);
	PlayerUtil::GetAlive(all, target);
}

void RedirUtil::RandTarget(Player* caster, const Redirect& redir, PlayerVec& target)
{
	TargetTeam team = redir.type == REDIR_ALL_MATE ? TEAM_MATE : TEAM_ENEMY;
	PlayerVec all, alive;
	PlayerUtil::GetMembers(caster, team, all);
	PlayerUtil::GetAlive(all, alive);
	Util::Rand(alive, target, redir.num, redir.max_selected);

}

} // namespace skill
