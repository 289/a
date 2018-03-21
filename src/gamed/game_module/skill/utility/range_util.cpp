#include "range_util.h"
#include "skill_templ.h"
#include "skill_range.h"
#include "obj_interface.h"
#include "player_util.h"
#include "util.h"

namespace skill
{

static void CheckPosValid(int8_t type, int8_t pos)
{
	if (type == RANGE_RANGE || type == RANGE_ALL)
	{
		assert(pos >= POS_1 && pos <= POS_CENTER);
	}
	else if (type == RANGE_LINE || type == RANGE_BULLET_LINE)
	{
		assert(pos >= LINE_FRONT && pos <= LINE_DOWN);
	}
	else
	{
		assert(pos >= POS_1 && pos <= POS_4);
	}
}

void RangeUtil::GetAttackRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	int8_t type = templ->range.type;
	CheckPosValid(type, pos);
	switch (type)
	{
	case RANGE_SINGLE:
		return SingleRange(templ, pos, players, target);
	case RANGE_RANGE:
		return RangeRange(templ, pos, players, target);
	case RANGE_ALL:
		return AllRange(templ, pos, players, target);
	case RANGE_LINE:
	case RANGE_BULLET_LINE:
		return LineRange(templ, pos, players, target);
	case RANGE_CHAIN:
		return ChainRange(templ, pos, players, target);
	default:
		assert(false);
		return;
	}
}

void RangeUtil::SingleRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	// 因为Range为SINGLE时，Target不可能为APPOINT
	// 因此当Target为非DEAD时，pos选中的人肯定血量大于0
	// 为DEAD时，则pos选中的人为濒死或者死亡
	Player* obj = players[pos];
	target.push_back(obj);
}

void RangeUtil::RangeRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	size_t size = 0;
	int32_t num = templ->range.num;
	Player* obj = NULL;
	if (pos != POS_CENTER)
	{
		obj = players[pos];
		target.push_back(obj);
		players.erase(players.begin() + pos);
		--num;
	}

	PlayerVec selected;
	// 不是死亡优先的情况则先优先选定可攻击，其次存活的
	if (templ->target.type != TARGET_DEAD)
	{
		if (PlayerUtil::GetCanAttacked(players, selected) > num)
		{
			Util::Rand(selected, target, num);
			return;
		}
		PlayerUtil::GetCanAction(players, selected);
		Util::Rand(selected, target, num);
		return;
	}
	// 死亡优先但选出了一个存活的，所以场上没有人死，随机选
	else if (PlayerUtil::CanAction(obj))
	{
		PlayerUtil::GetCanAction(players, selected);
		Util::Rand(selected, target, num);
		return;
	}
	// 场上存在至少一个死亡的情况
	else if (PlayerUtil::IsDead(obj))
	{
		if (PlayerUtil::GetDead(players, selected) > num)
		{
			Util::Rand(selected, target, num);
			return;
		}
		size = selected.size();
		for (size_t i  = 0; i < size; ++i)
		{
			target.push_back(selected[i]);
		}
		num -= size;
	}
	// 场上只有濒死的情况
	if (PlayerUtil::GetDying(players, selected) > num)
	{
		Util::Rand(selected, target, num);
		return;
	}
	size = selected.size();
	for (size_t i  = 0; i < size; ++i)
	{
		target.push_back(selected[i]);
	}
	num -= size;
	PlayerUtil::GetCanAction(players, selected);
	Util::Rand(selected, target, num);
	return;
}

void RangeUtil::AllRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	if (templ->target.type != TARGET_DEAD)
	{
		PlayerUtil::GetAlive(players, target);
	}
	else
	{
		target = players;
	}
}

static void LineFrontRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_2; ++i)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineBackRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_3; i<= POS_4; ++i)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineVMidRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_2; i<= POS_3; ++i)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineUpRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_2; i<= POS_4; i += 2)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineDownRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_3; i += 2)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineHMidRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_4; i += 3)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineTwoFrontRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_3; ++i)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineTwoBackRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_2; i<= POS_4; ++i)
	{
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineTwoUpRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_4; ++i)
	{
		if (i == POS_3)
		{
			continue;
		}
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

static void LineTwoDownRange(PlayerVec& players, PlayerVec& target)
{
	Player* obj = NULL;
	for (int32_t i = POS_1; i<= POS_4; ++i)
	{
		if (i == POS_2)
		{
			continue;
		}
		obj = players[i];
		if (PlayerUtil::IsAlive(obj))
		{
			target.push_back(obj);
		}
	}
}

void RangeUtil::LineRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	switch (pos)
	{
	case LINE_FRONT:
		return LineFrontRange(players, target);
	case LINE_BACK:
		return LineBackRange(players, target);
	case LINE_VMID:
		return LineVMidRange(players, target);
	case LINE_UP:
		return LineUpRange(players, target);
	case LINE_DOWN:
		return LineDownRange(players, target);
	case LINE_HMID:
		return LineHMidRange(players, target);
	case LINE_TWO_FRONT:
		return LineTwoFrontRange(players, target);
	case LINE_TWO_BACK:
		return LineTwoBackRange(players, target);
	case LINE_TWO_UP:
		return LineTwoUpRange(players, target);
	case LINE_TWO_DOWN:
		return LineTwoDownRange(players, target);
	default:
		return;
	}
}

void RangeUtil::ChainRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target)
{
	assert(templ->target.type != TARGET_DEAD);
	Player* obj = players[pos];
	assert(PlayerUtil::CanAction(obj));
	target.push_back(obj);

	int32_t num = templ->range.num;
	int32_t max = templ->range.max_selected;
	bool chain = templ->range.chain_param != 0;
	if (max == 1)
	{
		players.erase(players.begin() + pos);
	}
	--num;

	PlayerVec selected;
	if (PlayerUtil::GetCanAttacked(players, selected) >= num)
	{
		Util::Rand(selected, target, num, max, chain, obj);
		return;
	}

	PlayerUtil::GetCanAction(players, selected);
	Util::Rand(selected, target, num, max, chain, obj);
}

} // namespace skill
