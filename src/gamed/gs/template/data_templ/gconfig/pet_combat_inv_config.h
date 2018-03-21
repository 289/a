#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_COMBAT_INV_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_COMBAT_INV_CONFIG_H_

namespace dataTempl {

class PetCombatInvConfig
{
public:
	static const int kMaxCombatInvSize = 33;
	BoundArray<int32_t, kMaxCombatInvSize> combat_inv_cap_config; // 宠物出战位配置表，下标为0的元素置空。
	                                                              // 记数组下标为i，则i表示有多个出战位，对应的元素值为需要的宠物能量上限值。
														          // 数组combat_inv_cap_config的长度减一表示当前支持的宠物最大出站位，即最多可以携带多少宠物出战。
	

	NESTED_DEFINE(combat_inv_cap_config);

	bool CheckDataValidity() const
	{
		for (size_t i = 1; i < combat_inv_cap_config.size(); ++ i)
		{
			if (combat_inv_cap_config[i] <= combat_inv_cap_config[i-1])
				return false;
		}
		return true;
	}
};

} // namespace dataTempl

#endif
