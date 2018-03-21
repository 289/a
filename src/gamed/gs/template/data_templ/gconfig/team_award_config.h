#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_TEAM_AWARD_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_TEAM_AWARD_CONFIG_H_

namespace dataTempl {

class TeamAwardConfig
{
public:
	BoundArray<float, 4> exp_factor_array;   // 组队战斗奖励的经验校正系数
	BoundArray<float, 4> money_factor_array; // 组队战斗奖励的金钱校正系数

	NESTED_DEFINE(exp_factor_array, money_factor_array);

	bool CheckDataValidity() const
	{
		for (size_t i = 0; i < exp_factor_array.size(); ++ i)
			if (exp_factor_array.size() < 0.000001f)
				return false;

		for (size_t i = 0; i < money_factor_array.size(); ++ i)
			if (money_factor_array.size() < 0.000001f)
				return false;

		return true;
	}
};

} // namespace dataTempl

#endif
