#include "player_util.h"
#include "obj_interface.h"
#include "damage_msg.h"

namespace skill
{

bool PlayerUtil::IsAlive(const Player* player)
{
	return player != NULL && !player->IsDead();
}

bool PlayerUtil::CanAction(const Player* player)
{
	return player != NULL && !player->IsDead() && !player->IsDying();
}

bool PlayerUtil::CanAttacked(const Player* player)
{
	return CanAction(player) && player->CanAttacked();
}

bool PlayerUtil::IsDead(const Player* player)
{
	return player != NULL && player->IsDead();
}

bool PlayerUtil::IsDying(const Player* player)
{
	return player != NULL && player->IsDying();
}

int32_t PlayerUtil::GetHP(const Player* player)
{
	return player->GetProperty(PROP_INDEX_HP);
}

int32_t PlayerUtil::GetMaxHP(const Player* player)
{
	return player->GetProperty(PROP_INDEX_MAX_HP);
}

int32_t PlayerUtil::GetMembers(const Player* player, int8_t team, PlayerVec& members)
{
    if (team == TEAM_MATE)
    {
        player->GetTeamMate(members);
    }
    else if (team == TEAM_ENEMY)
    {
        player->GetEnemy(members);
    }
    else
    {
        player->GetGolem(members);
    }
	return members.size();
}

int32_t PlayerUtil::GetAllPlayers(const Player* player, PlayerVec& all)
{
    for (int32_t i = 0; i < 2; ++i)
    {
        PlayerVec tmp;
        if (i == 0)
        {
	        player->GetTeamMate(tmp);
        }
        else
        {
	        player->GetEnemy(tmp);
        }
        PlayerVec::const_iterator it = tmp.begin();
        for (; it != tmp.end(); ++it)
        {
            if (*it != NULL)
            {
                all.push_back(*it);
            }
        }
    }
	return all.size();
}

Player* PlayerUtil::FindPlayer(const Player* player, int64_t roleid)
{
	PlayerVec members;
	for (int8_t team = 0; team <= 2; ++team)
	{
		members.clear();
		GetMembers(player, team, members);
		PlayerVec::const_iterator it = members.begin();
		for (; it != members.end(); ++it)
		{
			Player* target = *it;
			if (target != NULL && target->GetId() == roleid)
			{
				//return target->GetOwner();
				return target;
			}
		}
	}
	return NULL;
}

typedef bool (*Func)(const Player* player);
static int32_t GetValidPlayer(const PlayerVec& src, PlayerVec& valid, Func func)
{
	valid.clear();
	PlayerVec::const_iterator it = src.begin();
	for (; it != src.end(); ++it)
	{
		Player* player = *it;
		if (func(player))
		{
			valid.push_back(player);
		}
	}
	return valid.size();
}

int32_t PlayerUtil::GetAlive(const PlayerVec& src, PlayerVec& alive)
{
	return GetValidPlayer(src, alive, IsAlive);
}

int32_t PlayerUtil::GetCanAction(const PlayerVec& src, PlayerVec& action)
{
	return GetValidPlayer(src, action, CanAction);
}

int32_t PlayerUtil::GetCanAttacked(const PlayerVec& src, PlayerVec& attacked)
{
	return GetValidPlayer(src, attacked, CanAttacked);
}

int32_t PlayerUtil::GetDead(const PlayerVec& src, PlayerVec& des)
{
	return GetValidPlayer(src, des, IsDead);
}

int32_t PlayerUtil::GetDying(const PlayerVec& src, PlayerVec& des)
{
	return GetValidPlayer(src, des, IsDying);
}

typedef void (gamed::ObjInterface::*HPFunc)(int32_t hp, int32_t attacker);
int32_t PlayerUtil::ChangeProp(Player* attacker, Player* defender, Damage& dmg)
{
	int32_t status = RECAST_NONE;
	int32_t attackerid = attacker == NULL ? 0 : attacker->GetId();
	int32_t change_value = dmg.value < 0 ? -1 * dmg.value : dmg.value;
	if (dmg.type <= ATTACK_VALUE || (dmg.type == CHANGE_POINT && dmg.prop == PROP_INDEX_HP))
	{
		assert(dmg.prop == PROP_INDEX_HP);
		HPFunc func = dmg.type <= ATTACK_VALUE ? &Player::DoDamage : &Player::DoHeal;
		if (dmg.type <= ATTACK_VALUE)
		{
			__PRINTF("DoDamage attacker=%d, defender=%d prop=%d value=%d", attackerid, defender->GetId(), dmg.prop, change_value);
		}
		else
		{
			__PRINTF("DoHeal attacker=%d, defender=%d prop=%d value=%d", attackerid, defender->GetId(), dmg.prop, change_value);
		}
		// 因为战斗此时状态还没有更新，所以不能通过Dying或Dead接口进行判断
		bool before = defender->GetProperty(dmg.prop) <= 0;
		(defender->*(func))(change_value, attackerid);
		bool after = defender->GetProperty(dmg.prop) <= 0;
		if (!before && after)
		{
			status |= RECAST_KILL;
		}
	}
	else if (dmg.value >= 0 && dmg.type == CHANGE_POINT)
	{
		__PRINTF("IncPropPoint attacker=%d, defender=%d prop=%d value=%d", attackerid, defender->GetId(), dmg.prop, change_value);
		defender->IncPropPoint(dmg.prop, change_value);
	}
	else if (dmg.value < 0 && dmg.type == CHANGE_POINT)
	{
		__PRINTF("DecPropPoint attacker=%d, defender=%d prop=%d value=%d", attackerid, defender->GetId(), dmg.prop, change_value);
		defender->DecPropPoint(dmg.prop, change_value);
	}
	else if (dmg.type == CHANGE_SCALE)
	{
		defender->Revive(change_value);
	}

	dmg.type = dmg.type == CHANGE_SCALE ? DAMAGE_SCALE : DAMAGE_POINT;
	return status;
}

} // namespace skill
