#include "math_types.h"


namespace gamed {

#define PI 3.14159265

namespace maths
{

	float calc_angle(const A2DVECTOR& a, const A2DVECTOR& b)
	{
		A2DVECTOR aa = a; aa.normalize();
		A2DVECTOR bb = b; bb.normalize();
		float angle = atan2(aa.x*bb.y - aa.y*bb.x, dot_product(aa,bb));
		if (fabs(angle) < STD_EPSINON) return 0.f;
		return angle;
	}

	int calc_direction(const A2DVECTOR& offset)
	{
		A2DVECTOR base(1, 0);
		float angle = calc_angle(base, offset);
		if (angle < 0) 
			angle += 2 * PI;

		float base_angle = PI / 8;
		int direction    = 0;
		while (angle > base_angle)
		{
			base_angle += PI / 4;
			++direction;
		}

		return direction % 8;
	}

	int calc_direction(const A2DVECTOR& dest, const A2DVECTOR& src)
	{
		A2DVECTOR offset = dest;
		offset -= src;
		return calc_direction(offset);
	}

	float dot_product(const A2DVECTOR& a, const A2DVECTOR& b)
	{
		return a.dot_product(b);
	}

	float squared_distance(const A2DVECTOR& a, const A2DVECTOR& b)
	{
		return a.squared_distance(b);
	}

} // maths

} // namespace gamed
