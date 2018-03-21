#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_LEVELUP_EXP_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_LEVELUP_EXP_CONFIG_H_


namespace dataTempl {

class PlayerLvlUpExpConfig
{
public:
	static const int kMaxLevelCount = 255;

	BoundArray<int64_t, kMaxLevelCount+1> lvlup_exp_tbl; // 升级所需经验，1~255级
	                                                 // 0下标留空，升级所需经验从1开始填值，0表示当前等级不可升级

	NESTED_DEFINE(lvlup_exp_tbl);

	bool CheckDataValidity() const
	{
		if (lvlup_exp_tbl.size() <= 0)
			return false;
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_LEVELUP_EXP_CONFIG_H_
