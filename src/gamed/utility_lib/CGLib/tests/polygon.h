#ifndef GAMED_CGLib_POLYGON_H_
#define GAMED_CGLib_POLYGON_H_

#include <assert.h>
#include <vector>


namespace CGLib {

const float STD_EPSINON = 0.00001f;

inline bool is_zero(double n)
{
	return (n > -STD_EPSINON) && (n < STD_EPSINON);
}

inline bool is_float_equal(float x, float y)
{
	return is_zero(y - x);
}

///
/// Point
///
class Point
{
public:
	Point(float a_x, float a_y)
		: x(a_x),
		  y(a_y)
	{ }

	Point()
		: x(0.f),
		  y(0.f)
	{ }

	float x;
	float y;
};

///
/// LineSeg
///
class LineSeg
{
public:
	LineSeg(const Point& s_pt, const Point& e_pt)
		: sp(s_pt),
		  ep(e_pt)
	{ }

	Point sp; // start point
	Point ep; // end point

	inline bool isHorizontal() const;
};

inline bool LineSeg::isHorizontal() const
{
	return is_float_equal(ep.y, sp.y);
}

///
/// Polygon
///
class Polygon
{
public:
	std::vector<Point> vertices;

	inline bool isValid() const;
};

inline bool Polygon::isValid() const
{
	return vertices.size() >= 3;
}

///
/// Rect
///
class Rect
{
public:
	Rect(const Point& a_p1, const Point& a_p2)
		: p1(a_p1),
		  p2(a_p2)
	{ }

	Rect()
		: p1(0.f, 0.f),
		  p2(0.f, 0.f)
	{ }

	Point p1;
	Point p2;
};

double CrossProduct(float x1, float y1, float x2, float y2)
{
	double res = (double)x1*y2 - (double)y1*x2;
	return res;
}

// TODO: ?????????
bool IsPointInRect(const Rect& rc, const Point& pt)
{
	double xr = (pt.x - rc.p1.x) * (pt.x - rc.p2.x);
	double yr = (pt.y - rc.p1.y) * (pt.y - rc.p2.y);

	return (xr <= 0.0f) && (yr <= 0.0f);
}

bool IsPointOnLineSegment(const LineSeg& line, const Point& pt)
{
	Rect rc;
	rc.p1 = line.sp;
	rc.p2 = line.ep;

	double cp = CrossProduct(line.ep.x - line.sp.x, line.ep.y - line.sp.y,
			                 pt.x - line.sp.x, pt.y - line.sp.y);

	return (IsPointInRect(rc, pt)) && is_zero(cp);
}

bool IsRectIntersect(const Rect& rc1, const Rect& rc2)
{
	return ( (std::max(rc1.p1.x, rc1.p2.x) >= std::min(rc2.p1.x, rc2.p2.x))
		  && (std::max(rc2.p1.x, rc2.p2.x) >= std::min(rc1.p1.x, rc1.p2.x))
		  && (std::max(rc1.p1.y, rc1.p2.y) >= std::min(rc2.p1.y, rc2.p2.y))
		  && (std::max(rc2.p1.y, rc2.p2.y) >= std::min(rc1.p1.y, rc1.p2.y)) );
}

bool IsLineSegExclusive(const LineSeg& line1, const LineSeg& line2)
{
	Rect rc1;
	rc1.p1 = line1.sp;
	rc1.p2 = line1.ep;

	Rect rc2;
	rc2.p1 = line2.sp;
	rc2.p2 = line2.ep;

	return IsRectIntersect(rc1, rc2);
}

bool IsLineSegIntersect(const LineSeg& line1, const LineSeg& line2)
{
	if (IsLineSegExclusive(line1, line2))
	{
		return false;
	}

	double p1xq = CrossProduct(line1.sp.x - line2.sp.x, line1.sp.y - line2.sp.y,
			                   line2.ep.x - line2.sp.x, line2.ep.y - line2.sp.y);

	double p2xq = CrossProduct(line1.ep.x - line2.sp.x, line1.ep.y - line2.sp.y,
			                   line2.ep.x - line2.sp.x, line2.ep.y - line2.sp.y);

	double q1xp = CrossProduct(line2.sp.x - line1.sp.x, line2.sp.y - line1.sp.y,
			                   line1.ep.x - line1.sp.x, line1.ep.y - line1.sp.y);

	double q2xp = CrossProduct(line2.ep.x - line1.sp.x, line2.ep.y - line1.sp.y,
			                   line1.ep.x - line1.sp.x, line1.ep.y - line1.sp.y);

	// 跨立实验
	return ( (p1xq * p2xq <= 0.0f) && (q1xp * q2xp <= 0.0f) );
}

bool IsPointInPolygon(const Polygon& py, const Point& pt, float infinite)
{
	assert(py.isValid()); // 只考虑正常的多边形

	int count = 0;
	LineSeg rayseg = LineSeg(pt, Point(infinite, pt.y)); // 射线L
	for (size_t i = 0; i < py.vertices.size(); ++i)
	{
		// 当前点和下一个点组成线段P1P2
		LineSeg edge = LineSeg(py.vertices[i], py.vertices[(i+1)%py.vertices.size()]);
		if (IsPointOnLineSegment(edge, pt))
		{
			return true;
		}

		if (!edge.isHorizontal())
		{
			if (is_float_equal(edge.sp.y, pt.y) && (edge.sp.y > edge.ep.y))
			{
				++count;
			}
			else if ((is_float_equal(edge.ep.y, pt.y)) && (edge.ep.y > edge.sp.y))
			{
				++count;
			}
			else
			{
				if (IsLineSegIntersect(edge, rayseg))
				{
					++count;
				}
			}
		}
	}

	return ((count % 2) == 1);
}

} // namespace CGLib

#endif // GAMED_CGLib_POLYGON_H_
