//#include "polygon.h"

#include <stdio.h>

#include "cglib.h"

#include "wykobi/wykobi.hpp"
#include "wykobi/wykobi_utilities.hpp"

using namespace wykobi;

using namespace CGLib;

int main()
{
	point2d<float> p1, p2, p3, p4, p5;
	p1.x = 1.f;
	p1.y = 1.f;
	p2.x = 2.f;
	p2.y = 1.f;
	p3.x = 2.f;
	p3.y = 2.f;
	p4.x = 1.f;
	p4.y = 2.f;
	p5.x = 1.5f;
	p5.y = 1.5f;

	polygon<float, 2> py;
	py.push_back(p1);
	py.push_back(p2);
	py.push_back(p3);
	py.push_back(p4);
	py.push_back(p5);

	point2d<float> pt;
	pt.x = 1.f;
	pt.y = 1.99f;
	if (is_convex_polygon(py))
	{
		if (point_in_convex_polygon(pt, py))
		{
			printf("point in polygon!\n");
		}
		else
		{
			printf("point is't in polygon!\n");
		}
	}
	else 
	{
		if (point_in_polygon(pt, py))
		{
			printf("111111111111\n");
		}
		else
		{
			printf("222222222222\n");
		}
	}


	/*
	while (true)
	{
	// random points
	std::vector<point2d<float> > points;
	points.push_back(p1);
	points.push_back(p1);
	points.push_back(p1);
	points.push_back(p1);
	points.push_back(p1);
	rectangle<float> rect;
	rect[0].x = 1;
	rect[0].y = 1;
	rect[1].x = 2;
	rect[1].y = 2;
	generate_random_points(rect, 5, points.begin());
	rect[1].y = 2;
	}
	*/

	Point2d pp1(1, 1);
	Point2d pp2(2, 1);
	Point2d pp3(2, 2);
	Point2d pp4(1, 2);
	Point2d pp5(3, 3);

	Polygon ppy;
	ppy.push_back(pp1);
	ppy.push_back(pp2);
	ppy.push_back(pp3);
	ppy.push_back(pp4);
	ppy.push_back(pp5);

	bool ret = ppy.is_valid();
	ret = ppy.is_convex_polygon();
	Point2d ppt(1.f, 1.99f);
	ret = ppy.point_in_polygon(ppt); 

	return 0;
}
