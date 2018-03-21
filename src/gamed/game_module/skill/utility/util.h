#ifndef SKILL_UTIL_H_
#define SKILL_UTIL_H_

#include <algorithm>
#include "shared/security/randomgen.h"

namespace skill
{

// 通用辅助函数
class Util
{
public:
	static int32_t Rand(int32_t lower, int32_t upper)
	{
		return shared::net::RandomGen::RandUniform(lower, upper);
	}

	static int32_t Rand()
	{
		return Rand(1, 10000);
	}

	template <class T>
	static void Rand(std::vector<T*>& src, std::vector<T*>& des, int32_t num, int32_t max = 1, bool chain = false, const T* first = NULL)
	{
		const T* prev_selected = first;
		size_t min_size = chain ? 1 : 0;
		while (num > 0 && src.size() > min_size)
		{
			int32_t rand_num = Rand(0, src.size() - 1);
			T* selected = src[rand_num];
			int32_t select_num = std::count(des.begin(), des.end(), selected);
			if (select_num == max)
			{
				src.erase(src.begin() + rand_num);
				continue;
			}
			if (chain && prev_selected == selected)
			{
				continue;
			}
			prev_selected = selected;
			des.push_back(selected);
			--num;
		}
	}
};

} // namespace skill

#endif // SKILL_UTIL_H_
