#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_VISION_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_VISION_CONFIG_H_

namespace dataTempl {

/**
 * 玩家瞄类视觉配置表,
 * 开启瞄类视觉消耗精力
 * EP: energy power
 */
class PlayerCatVisionConfig
{
public:
	static const int kMaxCatVisionLevel = 32;

	int32_t ep_speed_use; // 精力消耗速度(每秒消耗多少精力)
	int32_t ep_speed_gen; // 精力恢复速度(单位时间恢复多少精力)
	int32_t interval_gen; // 精力恢复间隔
	BoundArray<int32_t, kMaxCatVisionLevel> lvlup_exp_table; // 瞄类视觉升级经验表, 1~32级, 0下标留空
	
	NESTED_DEFINE(ep_speed_use, ep_speed_gen, interval_gen, lvlup_exp_table);

	bool CheckDataValidity() const
	{
		if (ep_speed_use <= 0 ||
			ep_speed_gen <= 0 ||
			interval_gen <= 0 ||
			lvlup_exp_table.size() <= 0)
			return false;
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_VISION_CONFIG_H_
