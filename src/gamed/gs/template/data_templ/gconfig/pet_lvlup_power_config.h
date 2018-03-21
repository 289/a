#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_POWER_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_POWER_CONFIG_H_

namespace dataTempl {

class PetLvlUpPowerConfig
{
public:
	int32_t init_pet_power_up_limit;  // 宠物能量上限的初始值
	int32_t max_pet_power_up_limit;   // 宠物能量上限的最大值
	int32_t pet_power_gen_speed;      // 宠物能量产生速度
	int32_t cash_need_on_lvlup;       // 能量上限提升需要的元宝个数
	int32_t item_need_on_lvlup;       // 能量上限提升需要的道具ID
	int32_t item_count_on_lvlup;      // 能量上限提升需要的道具数量
	int32_t cash_need_full_power;     // 一键加满当前能量的元宝个数

	struct PowerLvlUpEntry
	{
		int32_t probability;          // 升级概率
		int32_t power_inc_on_lvlup;   // 升级提升的能量值
		NESTED_DEFINE(probability, power_inc_on_lvlup);
	};

	BoundArray<PowerLvlUpEntry, 10> power_lvlup_table;

	NESTED_DEFINE(init_pet_power_up_limit,
				  max_pet_power_up_limit,
				  pet_power_gen_speed,
				  cash_need_on_lvlup,
				  item_need_on_lvlup,
				  item_count_on_lvlup,
				  cash_need_full_power,
				  power_lvlup_table);

	bool CheckDataValidity() const
	{
		if (init_pet_power_up_limit <= 0 ||
			max_pet_power_up_limit <= 0 ||
			pet_power_gen_speed <= 0 ||
			cash_need_on_lvlup <= 0 ||
			item_need_on_lvlup <= 0 ||
			item_count_on_lvlup < 1 ||
			cash_need_full_power <= 0)
			return false;

		for (size_t i = 0; i < power_lvlup_table.size(); ++ i)
		{
			if (power_lvlup_table[i].probability <= 0 ||
				power_lvlup_table[i].power_inc_on_lvlup <= 0)
				return false;
		}

		return true;
	}
};

} // namespace dataTempl

#endif
