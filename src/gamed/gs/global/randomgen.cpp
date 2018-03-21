#include "randomgen.h"

#include "shared/base/mutex.h"
#include "shared/security/randomgen.h"

#include "math_types.h"


namespace gamed {
namespace mrand {

using namespace shared;
using namespace shared::net;

static MutexLock s_random_lock;
static const size_t s_max_option_size = 128;
static const int32_t kProbTenThousand = 10000;
static const int32_t kProbOne         = 1;
	
void Init()
{
	RandomGen::Init();
}

int32_t Rand(int32_t lower, int32_t upper)
{
	MutexLockTimedGuard lock(s_random_lock);
	return RandomGen::RandUniform(lower, upper);
}

float RandF(float lower, float upper)
{
	MutexLockTimedGuard lock(s_random_lock);
	return RandomGen::RandUniform(lower, upper);
}

bool RandSelect(int32_t prob)
{
    if (prob == 0)
        return false;

	ASSERT(prob >= kProbOne && prob <= kProbTenThousand);

	int32_t option[2];
	option[0] = prob;
	option[1] = kProbTenThousand - prob;

	// not need to lock, RandSelect(a, b) is locked inside
	if (RandSelect(option, 2) == 0)
		return true;
	else
		return false;

	return false;
}

int32_t RandSelect(const int32_t* option, size_t size)
{
	ASSERT(size > 0 && size <= s_max_option_size);

	MutexLockTimedGuard lock(s_random_lock);
	int32_t p = RandomGen::RandUniform(kProbOne, kProbTenThousand);
	for (size_t i = 0; i < size; i++, option++)
	{
		int32_t tmp = *(int32_t*)option; ASSERT(tmp >= 0 && tmp <= kProbTenThousand);
		if (p <= tmp) return i;
		p -= tmp; ASSERT(p >= 0);
	}
	ASSERT(p == 0);
	return 0;
}

bool RandSelectF(float prob)
{
    if ((prob >= -STD_EPSINON) && (prob <= STD_EPSINON))
        return false;

	ASSERT(prob >= 0.f && prob <= 1.f);

	float option[2];
	option[0] = prob;
	option[1] = 1.f - prob;

	// not need to lock, RandSelectF(a, b) is locked inside
	if (RandSelectF(option, 2) == 0)
		return true;
	else
		return false;

	return false;
}

int32_t RandSelectF(const float* option, size_t size)
{
	ASSERT(size > 0 && size <= s_max_option_size);

	MutexLockTimedGuard lock(s_random_lock);
	float p = RandomGen::RandUniform();
	for (size_t i = 0; i < size; i++, option++)
	{
		float tmp = *(float*)option; ASSERT(tmp >= 0.f && tmp <= 1.f);
		if (p <= tmp) return i;
		p -= tmp; ASSERT(p >= 0.f);
	}
	ASSERT(p < 1e-5);
	return 0;
}

void Normalization(int32_t* prob, size_t size)
{
	double total = 0;
	for (size_t i = 0; i < size; ++i)
	{
		total += prob[i];
	}

	ASSERT(total > 0);
	int subTotal = 0;
	for (size_t i = 0; i < size - 1; ++i)
	{
		ASSERT(prob[i] > 0)
		prob[i]   = ((double)prob[i] / total) * (double)kProbTenThousand;
		subTotal += prob[i];
	}
	prob[size - 1] = kProbTenThousand - subTotal;
    ASSERT(prob[size - 1] >= 0);
}

} // namespace mrand
} // namespace gamed
