#ifndef GAMED_COMBAT_H_
#define GAMED_COMBAT_H_

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "combat_player.h"
#include "player_util.h"
#include "util.h"

namespace gamed
{

typedef std::vector<CombatPlayer*> CombatPlayerVec;

class Combat
{
public:
	static void Clear()
	{
		for (size_t i = 1; i <= 2; ++i)
		{
			CombatPlayerVec& player_vec = i == 1 ? team1 : team2;
			for (size_t j = 0; j < player_vec.size(); ++j)
			{
				CombatPlayer* player = player_vec[j];
				if (player != NULL)
				{
					delete player;
					player = NULL;
				}
			}
		}
	}

	static void AddPlayer(int8_t team, CombatPlayer* obj)
	{
		if (team == 1)
		{
			team1.push_back(obj);
		}
		else
		{
			team2.push_back(obj);
		}
	}
	static ObjInterface* GetPlayer(int8_t team, int8_t pos)
	{
		CombatPlayerVec& player_vec = team == 1 ? team1 : team2;
		for (size_t i = 0; i < player_vec.size(); ++i)
		{
			if (i == (size_t)pos)
			{
				return player_vec[i];
			}
		}
		return NULL;
	}

	static ObjInterface* GetPlayer(int64_t id)
	{
		int8_t team = id / 1000;
		int8_t pos = id % 1000;
		return GetPlayer(team, pos);
	}

	static void GetPlayer(int8_t team, std::vector<ObjInterface*>& players)
	{
		CombatPlayerVec& player_vec = team == 1 ? team1 : team2;
		for (size_t i = 0; i < player_vec.size(); ++i)
		{			
			ObjInterface* obj = player_vec[i];
			players.push_back(obj);
		}
	}

	static bool IsTeamAllDead(int8_t team)
	{
		const CombatPlayerVec& player_vec = team == 1 ? team1 : team2;
		for (size_t i = 0; i < player_vec.size(); ++i)
		{			
			ObjInterface* obj = player_vec[i];
			if (PlayerUtil::IsAlive(obj))
			{
				return false;
			}
		}
		return true;
	}

	static void Show()
	{
		for (size_t i = 1; i <= 2; ++i)
		{
			CombatPlayerVec& player_vec = i == 1 ? team1 : team2;
			__PRINTF("----------team=%ld----------", i);
			for (size_t j = 0; j < 4; ++j)
			{				
				ObjInterface* player = player_vec[j];
				if (player == NULL)
				{
					continue;
				}
				int64_t id = player->GetId();
				int32_t hp = player->GetProperty(PROP_INDEX_HP);
				int32_t con1 = player->GetProperty(PROP_INDEX_CON1);
				int32_t con2 = player->GetProperty(PROP_INDEX_CON2);
				int32_t pattack = player->GetProperty(PROP_INDEX_MAX_PHYSICAL_ATTACK);
				int32_t mattack = player->GetProperty(PROP_INDEX_MAX_MAGIC_ATTACK);
				int32_t pdefence = player->GetProperty(PROP_INDEX_MAX_PHYSICAL_DEFENCE);
				int32_t mdefence = player->GetProperty(PROP_INDEX_MAX_MAGIC_DEFENCE);
				int32_t speed = player->GetProperty(PROP_INDEX_MAX_ATTACK_SPEED);
				__PRINTF("player=%ld hp=%d con1=%d con2=%d pattack=%d mattack=%d pdefence=%d mdefence=%d speed=%d", id, hp, con1, con2, pattack, mattack, pdefence, mdefence, speed);
				BuffWrapper* buff = player->GetBuff();
				buff->Show();
			}
		}
	}
private:
	static CombatPlayerVec team1;
	static CombatPlayerVec team2;
};

} // namespace gamed

#endif // GAMED_COMBAT_H_
