#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_EXP_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PET_LEVELUP_EXP_CONFIG_H_

namespace dataTempl {

class PetLvlUpExpConfig
{
public:
	static const int kMaxLevelCount = 255;

	BoundArray<int64_t, kMaxLevelCount> exp_array; // 升级所需经验,1~255级,exp_array[i]表示从i级升级到i+1级需要的经验,exp_array[0]置空;

	NESTED_DEFINE(exp_array);

	bool CheckDataValidity() const
	{
		if (exp_array.size() <= 0)
			return false;
		return true;
	}
};

} // namespace dataTempl

#endif
