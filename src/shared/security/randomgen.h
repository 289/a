#ifndef SHARED_RANDOM_GEN_H
#define SHARED_RANDOM_GEN_H

#include <stdint.h>
#include "shared/security/random.h"

namespace shared
{
namespace net
{

class RandomGen
{
public:
	static void Init();
	// 生成[0,1)之间均匀分布的随机数
	static double RandUniform();
	// 生成[lower, upper]之间均匀分布的随机数
	static int32_t RandUniform(int32_t lower, int32_t upper);
	static float RandUniform(float lower, float upper);
	// 生成[lower, upper]之间正态分布的随机数
	static int32_t RandNormal(int32_t lower, int32_t upper);
	static float RandNormal(float lower, float upper);
private:
	static Random rand_;
};

} // namespace net
} // namespace shared

#endif // SHARED_RANDOM_GEN_H
