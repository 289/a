#ifndef TASK_BASIC_INFO_H_
#define TASK_BASIC_INFO_H_

#include "task_types.h"

namespace task
{

enum RatioType
{
	RATIO_GOLD,
	RATIO_EXP,
	RATIO_SCORE,
};

class RatioValue
{
public:
	RatioValue()
		: value(0), ratio(false)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t value;
	bool ratio;

	NESTED_DEFINE(value, ratio);
};
typedef RatioValue Gold;
typedef RatioValue Exp;
typedef RatioValue Score;

class Position
{
public:
	Position()
		: world_id(0), x(0), y(0), dir(0)
	{
	}

	inline bool CheckDataValidity() const;
	//inline bool IsEqual(int32_t world_id, float x, float y) const;

	int32_t world_id;
	float x;
	float y;
	int8_t dir;

	NESTED_DEFINE(world_id, x, y, dir);
};
typedef std::vector<Position> PositionVec;

class BaseInfo
{
public:
    BaseInfo()
        : id(0), value(0)
    {
    }

	inline bool CheckDataValidity() const;

    int32_t id;
    int32_t value;

    NESTED_DEFINE(id, value);
};

typedef BaseInfo TaskInfo;
typedef std::vector<TaskInfo> TaskInfoVec;
typedef BaseInfo InstanceInfo;
typedef std::vector<InstanceInfo> InstanceInfoVec;
typedef BaseInfo UIOpInfo;
typedef std::vector<UIOpInfo> UIOpVec;
typedef BaseInfo BuffOpInfo;
typedef std::vector<BuffOpInfo> BuffOpVec;
typedef BaseInfo FriendOpInfo;
typedef std::vector<FriendOpInfo> FriendOpVec;

class ReputationInfo
{
public:
    ReputationInfo()
        : id(0), value(0), op(0)
    {
    }

	inline bool CheckDataValidity() const;

    int32_t id;
    int32_t value;
    int8_t op;

    NESTED_DEFINE(id, value, op);
};
typedef std::vector<ReputationInfo> ReputationVec;

class TalentInfo
{
public:
    TalentInfo()
        : talent_gid(0), talent_id(0), value(0), op(0)
    {
    }

	inline bool CheckDataValidity() const;

    int32_t talent_gid;
    int32_t talent_id;
    int32_t value;
    int8_t op;

    NESTED_DEFINE(talent_gid, talent_id, value, op);
};
typedef std::vector<TalentInfo> TalentVec;

class PriorTrack
{
public:
    PriorTrack()
        : id(0), value(0)
    {
    }

    inline bool CheckDataValidity() const;

    int32_t id;
    int32_t value;
    std::string area_name;

    NESTED_DEFINE(id, value, area_name);
};

class CounterInfo
{
public:
    CounterInfo()
        : type(COUNTER_GLOBAL), op(COUNTER_OP_GREATER), id(0), world_id(0), value(0)
    {
    }

	inline bool CheckDataValidity() const;

    int8_t type;
    int8_t op;
    int32_t id;
    int32_t world_id;
    int32_t value;

    NESTED_DEFINE(type, op, id, world_id, value);
};
typedef std::vector<CounterInfo> CounterVec;

class ChatInfo
{
public:
	ChatInfo()
		: channel(CHAT_CHANNEL_WORLD)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t channel;
	std::string sender_name;
	std::string content;

	NESTED_DEFINE(channel, sender_name, content);
};
typedef std::vector<ChatInfo> ChatVec;

class DevelopInfo
{
public:
    DevelopInfo()
        : level(0), rank(0), needed_value(0)
    {
    }

	inline bool CheckDataValidity() const;

    int16_t level;
    int16_t rank;
    int16_t needed_value;

    NESTED_DEFINE(level, rank, needed_value);
};

class PetInfo : public DevelopInfo
{
public:
    PetInfo()
        : blevel(0)
    {
    }

	inline bool CheckDataValidity() const;

    int16_t blevel;

    NESTED_DEFINE(level, blevel, rank, needed_value);
};
typedef std::vector<PetInfo> PetVec;

class DlgTaskInfo
{
public:
    DlgTaskInfo()
        : pos(DLG_POS_RIGHT), time(0)
    {
    }

	inline bool CheckDataValidity() const;

    int8_t pos;
    int32_t time;
    std::string info;

    NESTED_DEFINE(pos, time, info);
};
typedef std::vector<DlgTaskInfo> DlgTaskInfoVec;

typedef DevelopInfo CardInfo;
typedef std::vector<CardInfo> CardVec;

typedef DevelopInfo EnhanceInfo;
typedef std::vector<EnhanceInfo> EnhanceVec;

typedef DevelopInfo EquipInfo;
typedef std::vector<EquipInfo> EquipVec;

class CombatValueInfo
{
public:
    CombatValueInfo()
        : op(VALUE_OP_LESS), value(0)
    {
    }

    inline bool CheckDataValidity() const;

    int8_t op;
    int32_t value;

    NESTED_DEFINE(op, value);
};
typedef std::vector<CombatValueInfo> CombatValueVec;

// 内联函数实现
inline bool RatioValue::CheckDataValidity() const
{
	return value >= 0;
}

inline bool Position::CheckDataValidity() const
{
	return world_id >= 0 && dir >= DIR_RIGHT && dir <= DIR_RIGHT_DOWN;
}

//inline bool Position::IsEqual(int32_t world_id, float x, float y) const
//{
	//int32_t x_diff = (int32_t)(this->x - x);
	//int32_t y_diff = (int32_t)(this->y - y);
	//return this->world_id == world_id && x_diff == 0 && y_diff == 0;
//}

inline bool BaseInfo::CheckDataValidity() const
{
	return id >= 0;
}

inline bool ReputationInfo::CheckDataValidity() const
{
    CHECK_INRANGE(op, REPUTATION_OP_EQUAL_GREATER, REPUTATION_OP_DEC)
    return id >= 0 && value >= 0;
}

inline bool TalentInfo::CheckDataValidity() const
{
    CHECK_INRANGE(op, VALUE_OP_LESS, VALUE_OP_GREATER)
    return talent_gid >= 0 && talent_id >= 0 && value >= 0;
}

inline bool PriorTrack::CheckDataValidity() const
{
    return id >= 0;
}

inline bool CounterInfo::CheckDataValidity() const
{
    CHECK_INRANGE(type, COUNTER_GLOBAL, COUNTER_PLAYER)
    CHECK_INRANGE(op, COUNTER_OP_GREATER, COUNTER_OP_UNLOAD)
    return id >= 0 && world_id >= 0;
}

inline bool ChatInfo::CheckDataValidity() const
{
	return channel >= CHAT_CHANNEL_WORLD && channel <= CHAT_CHANNEL_REGION;
}

inline bool DevelopInfo::CheckDataValidity() const
{
    return level >= 0 && rank >= 0 && needed_value >= 0;
}

inline bool PetInfo::CheckDataValidity() const
{
    return level >= 0 && blevel >= 0 && rank >= 0 && needed_value >= 0;
}

inline bool DlgTaskInfo::CheckDataValidity() const
{
	CHECK_INRANGE(pos, DLG_POS_RIGHT, DLG_POS_BOTTOM)
    return time >= 0;
}

inline bool CombatValueInfo::CheckDataValidity() const
{
    CHECK_INRANGE(op, VALUE_OP_LESS, VALUE_OP_GREATER)
    return value >= 0;
}

} // namespace task

#endif // TASK_BASIC_INFO_H_
