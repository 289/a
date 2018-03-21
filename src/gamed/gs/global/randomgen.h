#ifndef GAMED_GS_GLOBAL_RANDOMGEN_H_
#define GAMED_GS_GLOBAL_RANDOMGEN_H_

#include <stdlib.h>


namespace gamed {
namespace mrand {

	// 初始化随机算子
	void    Init();

	/**
	 * @brief Rand 
	 *    生成[lower, upper]之间均匀分布的随机数
	 */
	int32_t Rand(int32_t lower, int32_t upper);
	float   RandF(float lower, float upper);


	/**
	 * @brief RandSelect 
	 *  （1）给出概率参数，返回值表示是否中签
	 *  （2）输入概率是1~10000之间的整型(万分数)
	 */
	bool    RandSelect(int32_t prob);
	/**
	 * @brief RandSelect 
	 *  （1）给出概率数组为参数，返回第几项被选中或没有被选中
	 *  （2）输入概率是1~10000之间的整型(万分数)
	 */
	int32_t RandSelect(const int32_t* option, size_t size);


	/**
	 * @brief RandSelectF 
	 *  （1）给出概率参数，返回值表明是否中签
	 *  （2）输入概率是0~1之间的浮点数
	 */
	bool    RandSelectF(float prob);
	/**
	 * @brief RandSelectF 
	 *  （1）给出概率数组为参数，返回第几项被选中或没有被选中
	 *  （2）输入概率是0~1之间的浮点数，TODO: 最多支持128项
	 */
	int32_t RandSelectF(const float* option, size_t size);


    /**
     * @brief Normalization 
     *  （1）概率归一化，万分数
     */
	void Normalization(int32_t* prob, size_t size);
	
} // namespace mrand
} // namespace gamed

#endif // GAMED_GS_GLOBAL_RANDOMGEN_H_
