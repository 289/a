#include <float.h>
#include "target_util.h"
#include "skill_target.h"
#include "obj_interface.h"
#include "player_util.h"
#include "util.h"

namespace skill
{

// 由于存在ATB暂停，所以在存在随机选择的情况下，
// 都是优先选择可以攻击的对象作为目标
int8_t TargetUtil::GetCastPos(Player* caster, int8_t team, const SkillTarget& target, const PlayerVec& players)
{
	switch (target.type)
	{
	case TARGET_APPOINT:
		return Appoint(team, target.params, players);
	case TARGET_RANDOM:
		return Random(team, target.params, players);
	case TARGET_SEQUENCE:
		return Sequence(team, target.params, players);
	case TARGET_HP:
		return HP(team, target.params, players);
	case TARGET_LINE:
		return Line(team, target.params, players);
	case TARGET_CLASS:
		return Class(team, target.params, players);
	case TARGET_POSITION:
		return Position(team, target.params, players);
	case TARGET_DEAD:
		return Dead(team, target.params, players);
	default:
		return caster->GetPos();
	}
}

// 现在返回的位置对应的player不一定存活
int8_t TargetUtil::Appoint(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	return params[0];
}

static int8_t RandPos(const PlayerVec& players)
{
	int32_t num = Util::Rand(0, players.size() - 1);
	return players[num]->GetPos();
}

// 返回的player肯定存活
int8_t TargetUtil::Random(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	PlayerVec selected;
	if (PlayerUtil::GetCanAttacked(players, selected) == 0)
	{
		if (PlayerUtil::GetCanAction(players, selected) == 0)
		{
			return POS_INVALID;
		}
	}
	return RandPos(selected);
}

// 返回的player肯定存活
int8_t TargetUtil::Position(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	PlayerVec row_players = players;
	int8_t end = params[0] == FRONT_ROW ? POS_4 : POS_1;
	row_players.erase(row_players.begin() + end);

	PlayerVec action;
	if (PlayerUtil::GetCanAction(row_players, action) == 0)
	{
		return PlayerUtil::CanAction(players[end]) ? end : POS_INVALID;
	}

	PlayerVec attack;
	int32_t attack_size = PlayerUtil::GetCanAttacked(action, attack);
	return attack_size != 0 ? RandPos(attack) : RandPos(action);
}

// 返回的player肯定存活
int8_t TargetUtil::Sequence(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	PlayerVec action;
	return PlayerUtil::GetCanAction(players, action) ? action[0]->GetPos() : POS_INVALID;
}

// 返回的player肯定存活
typedef bool (*HPFunc)(const Player* player);
int8_t TargetUtil::HP(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	int32_t type = params[0];
	int8_t pos = POS_INVALID;
	float comp = type % 2 == 0 ? FLT_MAX : 0;
    HPFunc func = team == TEAM_MATE ? PlayerUtil::IsAlive : PlayerUtil::CanAction;
	PlayerVec::const_iterator it = players.begin();
	for (size_t i = 0; i < players.size(); ++i)
	{
		const Player* obj = players[i];
		if (func(obj))
		{
			float factor = type < HP_SCALE_MIN ? 1.0 : PlayerUtil::GetMaxHP(obj);
			float value = PlayerUtil::GetHP(obj) / factor;
			bool match = type % 2 == 0 ? value < comp : value > comp;
			if (match)
			{
				comp = value;
				pos = i;
			}
		}
	}
	return pos;
}

// 返回的player肯定存活
// 不过这里如果存在匹配的职业则没有随机而是直接返回的第一个（这里看策划情况修改）
int8_t TargetUtil::Class(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	PlayerVec action;
	if (PlayerUtil::GetCanAction(players, action) == 0)
	{
		return POS_INVALID;
	}

	for (size_t i = 0; i < action.size(); ++i)
	{
		const Player* obj = action[i];
		int32_t role_class = obj->GetRoleClass();
		ParamVec::const_iterator it = find(params.begin(), params.end(), role_class);
		if (it != params.end())
		{
			return obj->GetPos();
		}
	}

	PlayerVec attack;
	int32_t attack_size = PlayerUtil::GetCanAttacked(action, attack);
	return attack_size != 0 ? RandPos(attack) : RandPos(action);
}

static int8_t LineFront(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_1]) || PlayerUtil::IsAlive(players[POS_2]))
	{
		return LINE_FRONT;
	}
	if (PlayerUtil::IsAlive(players[POS_3]) || PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_BACK;
	}
	return POS_INVALID;
}

static int8_t LineBack(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_3]) || PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_BACK;
	}
	if (PlayerUtil::IsAlive(players[POS_1]) || PlayerUtil::IsAlive(players[POS_2]))
	{
		return LINE_FRONT;
	}
	return POS_INVALID;
}

static int8_t LineVMid(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_2]) || PlayerUtil::IsAlive(players[POS_3]))
	{
		return LINE_VMID;
	}
	int8_t pos = POS_INVALID;
	if (PlayerUtil::IsAlive(players[POS_1]))
	{		
		pos = LINE_FRONT;
	}
	if (PlayerUtil::IsAlive(players[POS_4]) && pos == POS_INVALID)
	{
		return LINE_BACK;
	}
	else if (PlayerUtil::IsAlive(players[POS_4]) && pos != POS_INVALID)
	{
		return Util::Rand(0, 1) == 0 ? LINE_FRONT : LINE_BACK;
	}
	return pos;
}

static int8_t LineUp(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_2]) || PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_UP;
	}
	if (PlayerUtil::IsAlive(players[POS_1]) || PlayerUtil::IsAlive(players[POS_3]))
	{
		return LINE_DOWN;
	}
	return POS_INVALID;
}

static int8_t LineDown(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_1]) || PlayerUtil::IsAlive(players[POS_3]))
	{
		return LINE_DOWN;
	}
	if (PlayerUtil::IsAlive(players[POS_2]) || PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_UP;
	}
	return POS_INVALID;
}

static int8_t LineHMid(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_1]) || PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_HMID;
	}
	int8_t pos = POS_INVALID;
	if (PlayerUtil::IsAlive(players[POS_2]))
	{		
		pos = LINE_UP;
	}
	if (PlayerUtil::IsAlive(players[POS_3]) && pos == POS_INVALID)
	{
		return LINE_DOWN;
	}
	else if (PlayerUtil::IsAlive(players[POS_3]) && pos != POS_INVALID)
	{
		return Util::Rand(0, 1) == 0 ? LINE_UP : LINE_DOWN;
	}
	return pos;
}

static int8_t LineTwoFront(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_1]))
	{
		return LINE_TWO_FRONT;
	}
	if (!PlayerUtil::IsAlive(players[POS_2]) 
		&& !PlayerUtil::IsAlive(players[POS_3]) 
		&& !PlayerUtil::IsAlive(players[POS_4]))
	{
		return POS_INVALID;
	}
	return LINE_TWO_BACK;
}

static int8_t LineTwoBack(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_4]))
	{
		return LINE_TWO_BACK;
	}
	if (!PlayerUtil::IsAlive(players[POS_1]) 
		&& !PlayerUtil::IsAlive(players[POS_2]) 
		&& !PlayerUtil::IsAlive(players[POS_3]))
	{
		return POS_INVALID;
	}
	return LINE_TWO_FRONT;
}

static int8_t LineTwoUp(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_2]))
	{
		return LINE_TWO_UP;
	}
	if (!PlayerUtil::IsAlive(players[POS_1]) 
		&& !PlayerUtil::IsAlive(players[POS_3]) 
		&& !PlayerUtil::IsAlive(players[POS_4]))
	{
		return POS_INVALID;
	}
	return LINE_TWO_DOWN;
}

static int8_t LineTwoDown(const PlayerVec& players)
{
	if (PlayerUtil::IsAlive(players[POS_3]))
	{
		return LINE_TWO_DOWN;
	}
	if (!PlayerUtil::IsAlive(players[POS_1]) 
		&& !PlayerUtil::IsAlive(players[POS_2]) 
		&& !PlayerUtil::IsAlive(players[POS_4]))
	{
		return POS_INVALID;
	}
	return LINE_TWO_UP;
}

static int8_t RandLineType(int8_t type)
{
	if (type >= LINE_FRONT && type <= LINE_DOWN)
	{
		return type;
	}
	else if (type == LINE_RAND_VERTICAL)
	{
		return LINE_FRONT + 2 * Util::Rand(0, 2);
	}
	else if (type == LINE_RAND_HORIZONTAL)
	{
		return LINE_UP + 2 * Util::Rand(0, 2);
	}
	else if (type == LINE_RAND_VTWO)
	{
		return Util::Rand(0, 1) == 0 ? LINE_TWO_FRONT : LINE_TWO_BACK;
	}
	else if (type == LINE_RAND_HTWO)
	{
		return Util::Rand(0, 1) == 0 ? LINE_TWO_UP : LINE_TWO_DOWN;
	}
	else
	{
		return POS_INVALID;
	}
}

// 返回的是策划规定的几个贯穿线位置
// 最后得到的上面的player可能存活或者濒死
int8_t TargetUtil::Line(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	int8_t type = RandLineType(params[0]);
	//int8_t type = params[0];
	switch (type)
	{
	case LINE_FRONT:
		return LineFront(players);
	case LINE_BACK:
		return LineBack(players);
	case LINE_VMID:
		return LineVMid(players);
	case LINE_UP:
		return LineUp(players);
	case LINE_DOWN:
		return LineDown(players);
	case LINE_HMID:
		return LineHMid(players);
	case LINE_TWO_FRONT:
		return LineTwoFront(players);
	case LINE_TWO_BACK:
		return LineTwoBack(players);
	case LINE_TWO_UP:
		return LineTwoUp(players);
	case LINE_TWO_DOWN:
		return LineTwoDown(players);
	default:
		assert(false);
		return POS_INVALID;
	}
}

// 返回的player可能死亡，濒死，存活
int8_t TargetUtil::Dead(int8_t team, const ParamVec& params, const PlayerVec& players)
{
	PlayerVec selected;
	if (PlayerUtil::GetDead(players, selected) == 0)
	{
		if (PlayerUtil::GetDying(players, selected) == 0)
		{
			return Random(team, params, players);
		}
	}
	return RandPos(selected);
}

} // namespace skill
