#ifndef __PF_POINT_H__
#define __PF_POINT_H__

#include <vector>
#include <math.h>

namespace pathfinder
{

template <class T>
class Point
{
public:
	T x, z;

public:
	Point():x(-1),z(-1)
	{
	}
	Point(const T& __x, const T& __z)
	{
		x = __x;
		z = __z;
	}

	friend bool operator != (const Point& pt1, const Point& pt2) { return pt1.x != pt2.x || pt1.z != pt2.z; }
	friend bool operator == (const Point& pt1, const Point& pt2) { return pt1.x == pt2.x && pt1.z == pt2.z; }
	friend Point operator + (const Point& pt1, const Point& pt2) { return Point(pt1.x+pt2.x, pt1.z+pt2.z); }
	friend Point operator - (const Point& pt1, const Point& pt2) { return Point(pt1.x-pt2.x, pt1.z-pt2.z); }

	Point operator + () const { return *this; }
	Point operator - () const { return Point(-x, -z); }

	Point& operator =  (const Point& pt) { x = pt.x; z = pt.z; return *this; }
	Point& operator += (const Point& pt) { x += pt.x; z += pt.z; return *this; }
	Point& operator -= (const Point& pt) { x -= pt.x; z -= pt.z; return *this; }

	void Offset(const T& ox, const T& oz)
	{
		x += ox;
		z += oz;
	}

	void Normalization()
	{
		float len = Magnitude();
		if (len > 0.0f)
		{
			x = x / len;
			z = z / len;
		}
	}

	T Magnitude()
	{
		return sqrtf(x * x + z * z);
	}

	T SquaredMagnitude()
	{
		return (x * x + z * z);
	}
};

typedef Point<float> PF2DVECTOR;// 大世界坐标

}; // namespace pathfinder

#endif // __PF_POINT_H__
