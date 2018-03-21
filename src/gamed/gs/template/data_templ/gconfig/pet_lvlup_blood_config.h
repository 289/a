#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_BLOOD_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_BLOOD_CONFIG_H_

namespace dataTempl {

class PetLvlUpBloodConfig
{
public:
	static const int kMaxLevelCount = 255;

	struct LvlupNode
	{
		TemplID lvlup_item_id;       // 升级消耗的道具ID
		int32_t lvlup_item_count;    // 升级消耗的道具数量
		int32_t lvlup_money;         // 升级消耗的金钱数
		int32_t lvlup_prob;          // 升级成功率(万分数)
		TemplID inc_prob_item_id;    // 提升升级概率的道具ID,升级概率提升1%
		int32_t inc_prob_item_count; // 提升升级概率的道具数量

		NESTED_DEFINE(lvlup_item_id,
				      lvlup_item_count,
					  lvlup_money,
					  lvlup_prob,
					  inc_prob_item_id,
					  inc_prob_item_count);
	};

	BoundArray<LvlupNode, kMaxLevelCount> lvlup_array; // 升级所需经验,1~255级,lvlup_array[i]表示从i级升级到i+1级的消耗,lvlup_array[0]置空;

	NESTED_DEFINE(lvlup_array);

	bool CheckDataValidity() const
	{
		if (lvlup_array.size() <= 0)
			return false;

		for (size_t i = 1; i < lvlup_array.size(); ++ i)
		{
			const LvlupNode& node = lvlup_array[i];

			if (node.lvlup_item_id < 0 ||
				node.lvlup_item_count < 0 ||
				node.lvlup_money < 0 ||
				node.lvlup_prob <= 0 ||
				node.inc_prob_item_id < 0 ||
				node.inc_prob_item_count < 0)
				return false;
		}
		return true;
	}
};

} // namespace dataTempl

#endif
