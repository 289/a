#include <limits.h>
#include "logic_purify.h"
#include "effect_logic.h"
#include "obj_interface.h"
#include "buff_wrapper.h"
#include "effect_templ.h"
#include "util.h"

namespace skill
{

bool LogicPurify::Purify(InnerMsg& msg)
{
	const EffectLogic::ParamVec& param_vec = logic_->params;
	int32_t max_count = atoi(param_vec[0].c_str());
	if (max_count < 0)
	{
		max_count = INT_MAX;
	}

	// 随机选出满足要求的待清除Buff
	std::vector<Buff*> pbuff_vec;
	BuffWrapper* buff = trigger_->GetBuff();
	BuffVec& buff_vec = buff->GetBuff();
	BuffVec::iterator bit = buff_vec.begin();
	for (; bit != buff_vec.end(); ++bit)
	{
		const EffectTempl* effect = bit->effect_;
		for (size_t i = 1; i < param_vec.size(); ++i)
		{
			if (effect->effect_group.count(atoi(param_vec[i].c_str())))
			{
				pbuff_vec.push_back(&(*bit));
				break;
			}
		}
	}
	int32_t delta = (int32_t)pbuff_vec.size() - max_count;
	while (delta-- > 0)
	{
		int32_t index = Util::Rand(0, pbuff_vec.size() - 1);
		pbuff_vec.erase(pbuff_vec.begin() + index);		
	}

	// 清除Buff
	bit = buff_vec.begin();
	for (; bit != buff_vec.end();)
	{
		if (find(pbuff_vec.begin(), pbuff_vec.end(), &(*bit)) != pbuff_vec.end())
		{
			bit->Detach(msg);
			bit = buff_vec.erase(bit);
		}
		else
		{
			++bit;
		}
	}
	return true;
}

} // namespace skill
