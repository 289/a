#ifndef GAMED_GS_GLOBAL_MATH_TYPES_H_
#define GAMED_GS_GLOBAL_MATH_TYPES_H_

/*****************************
 * [coordinate system] 
 * - Mark positive direction
 *
 *  *-----------> 
 *  |       x
 *  |
 *  | y
 *  | 
 *  V 
 *
 * ***************************/

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <limits> // std::numeric_limits
#include <string>


namespace gamed {

#define LOW_16_MASK 0x0000FFFF
#define HIGH_16_MASK 0xFFFF0000

const float STD_EPSINON = 0.00001f;

inline int64_t makeInt64(int32_t high, int32_t low)
{
	return ((int64_t)(((int32_t)(low & 0xffffffff)) | ((int64_t)((int32_t)(high & 0xffffffff))) << 32));
}

inline int32_t high32bits(int64_t ll)
{
	return ((int32_t)((ll >> 32) & 0xffffffff));
}

inline int32_t low32bits(int64_t ll)
{
	return ((int32_t)(ll & 0xffffffff));
}

inline const std::string itos(int64_t n)
{
	const int max_size = std::numeric_limits<int64_t>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
	char buffer[max_size] = {0};
	snprintf(buffer, max_size, "%ld", n);
	return std::string(buffer);
}

inline const std::string ftos(double n)
{
	const int max_size = std::numeric_limits<double>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
	char buffer[max_size] = {0};
	snprintf(buffer, max_size, "%f", n);
	return std::string(buffer);
}


/**
 * @brief prob_to_float 
 *    概率（万分数）转为0~1的浮点型，参数的合法性有调用者负责
 */
inline float prob_to_float(size_t prob)
{
	return (float)prob / 10000;
}

struct rect
{
	inline rect(float l, float t, float r, float b)
		: left(l), top(t), right(r), bottom(b)
	{ }

	inline rect()
	{ }

	inline bool IsIn(float x, float y) const 
	{
		return x >= left && x < right && y >= top && y < bottom;
	}

	inline bool IsIn(const rect &rt) const 
	{ 
		return rt.left >=left && rt.right <=right && rt.top >=top && rt.bottom <= bottom; 
	}

	inline bool IsOut(float x, float y) const
	{
		return x < left || x >= right || y < top || y >= bottom;
	}

	inline float Width() const 
	{
		return right - left;
	}

	inline float Height() const 
	{
		return bottom - top;
	}

    inline bool IsOverlap(const rect & rt) const
	{
		if(rt.left >= right || rt.top >= bottom || rt.bottom <= top || rt.right <= left) 
			return false;

		return true;
	}

	float left;     // x min
	float top;      // y min
	float right;    // x max
	float bottom;   // y max
};

#define INVALID_POS_COORD (-4096.f)

// copyable class
class A2DVECTOR
{
public:
	A2DVECTOR() 
		: x(INVALID_POS_COORD),
		  y(INVALID_POS_COORD)
	{ }

	A2DVECTOR(float x, float y)
		: x(x),
		  y(y)
	{ }

	A2DVECTOR(const A2DVECTOR& rhs)
		: x(rhs.x),
		  y(rhs.y)
	{ }

	A2DVECTOR& operator+=(const A2DVECTOR& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	inline A2DVECTOR& operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	inline const A2DVECTOR& operator-=(const A2DVECTOR& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	inline A2DVECTOR operator-(const A2DVECTOR& rhs)
	{
		A2DVECTOR ret(*this);
		ret -= rhs;
		return ret;
	}

	inline float squared_distance(const A2DVECTOR& pos) const
	{
		float disx = x - pos.x;
		float disy = y - pos.y;
		return disx*disx + disy*disy;
	}

	inline float squared_magnitude() const
	{
		return x*x + y*y;
	}

	inline float dot_product(const A2DVECTOR & pos) const
	{
		return x*pos.x + y*pos.y;
	}

	inline void normalize()
	{
		float mag = 1.f/sqrt(x*x + y*y);
		x *= mag;
		y *= mag;
	}

	float x;
	float y;
};

inline A2DVECTOR operator-(const A2DVECTOR& lhs, const A2DVECTOR& rhs)
{
	A2DVECTOR result;
	result  = lhs;
	result -= rhs;
	return result;
}

inline A2DVECTOR operator+(const A2DVECTOR& lhs, const A2DVECTOR& rhs)
{
	A2DVECTOR result;
	result  = lhs;
	result += rhs;
	return result;
}

inline A2DVECTOR operator*(const A2DVECTOR& lhs, double scale)
{
	A2DVECTOR result;
	result  = lhs;
	result *= scale;
	return result;
}

namespace maths
{
	/**
	 * @brief calc_angle 
	 * @return -PI ~ PI的浮点型
	 */
	float calc_angle(const A2DVECTOR& a, const A2DVECTOR& b);


	/**
	 * @brief calc_direction 
	 * @return 0~7 表示八个方向，0是向右的x轴正向，逆时针方向顺序
	 */
	int   calc_direction(const A2DVECTOR& offset);
	int   calc_direction(const A2DVECTOR& dest, const A2DVECTOR& src);


	/**
	 * @brief dot_product 
	 *    两个向量点乘
	 */
	float dot_product(const A2DVECTOR& a, const A2DVECTOR& b);


	/**
	 * @brief squared_distance 
	 *    两点的距离平方
	 */
	float squared_distance(const A2DVECTOR& a, const A2DVECTOR& b);
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_MATH_TYPES_H_
