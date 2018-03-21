#include "skill_target.h"

namespace skill
{

typedef bool (*Func)(const ParamVec& params);
// 无参数类型的检查函数
static bool NullParamValid(const ParamVec& params)
{
	return true; 
}

static bool AppointParamValid(const ParamVec& params)
{
	if (params.size() != 1)
	{
		return false;
	}
	return params[0] >= POS_1 && params[0] <= POS_CENTER;
}

static bool HPParamValid(const ParamVec& params)
{
	if (params.size() != 1)
	{
		return false;
	}
	return params[0] >= HP_VALUE_MIN && params[0] <= HP_SCALE_MAX;
}

static bool LineParamValid(const ParamVec& params)
{
	if (params.size() != 1)
	{
		return false;
	}
	return params[0] >= LINE_FRONT && params[0] <= LINE_RAND_HTWO;
}

static bool ClassParamValid(const ParamVec& params)
{
	for (size_t i = 0; i < params.size(); ++i)
	{
		if (params[0] <= CLS_NONE || params[0] >= CLS_MAXCLS_LABEL)
		{
			return false;
		}
	}
	return true;
}

static bool PosParamValid(const ParamVec& params)
{
	if (params.size() != 1)
	{
		return false;
	}
	return params[0] >= FRONT_ROW && params[0] <= BACK_ROW;
}

static Func GetCheckFunc(int8_t type)
{
	switch (type)
	{
	case TARGET_APPOINT:
		return AppointParamValid;
	case TARGET_HP:
		return HPParamValid;
	case TARGET_LINE:
		return LineParamValid;
	case TARGET_CLASS:
		return ClassParamValid;
	case TARGET_POSITION:
		return PosParamValid;
	default:
		return NullParamValid;
	}
}

bool SkillTarget::CheckDataValidity() const
{
	CHECK_INRANGE(type, TARGET_SELF, TARGET_TYPE_MAX)
	Func func = GetCheckFunc(type);
	return func(params);
}

} // namespace skill
