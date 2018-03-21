#include "cglib.h"

#include "wykobi/wykobi.hpp"
#include "wykobi/wykobi_utilities.hpp"


namespace CGLib {

void init_random_func(const custom_random func)
{
	wykobi::init_random_func(func);
}

Point2d generate_random_point(Point2d ptA, Point2d ptB, Point2d ptC)
{
	wykobi::point2d<float> tmp_pt;
	wykobi::triangle<float,2> tmp_triangle;
	tmp_triangle[0].x = ptA.x();
	tmp_triangle[0].y = ptA.y();
	tmp_triangle[1].x = ptB.x();
	tmp_triangle[1].y = ptB.y();
	tmp_triangle[2].x = ptC.x();
	tmp_triangle[2].y = ptC.y();
	tmp_pt = generate_random_point(tmp_triangle);
	
	Point2d ret_point(tmp_pt.x, tmp_pt.y);
	return ret_point;
}

Point2d generate_random_point(Point2d ptA, Point2d ptB, Point2d ptC, Point2d ptD)
{
    wykobi::point2d<float> tmp_pt;
    wykobi::quadix<float,2> tmp_quadix;
    tmp_quadix[0].x = ptA.x();
	tmp_quadix[0].y = ptA.y();
	tmp_quadix[1].x = ptB.x();
	tmp_quadix[1].y = ptB.y();
	tmp_quadix[2].x = ptC.x();
	tmp_quadix[2].y = ptC.y();
    tmp_quadix[3].x = ptD.x();
    tmp_quadix[3].y = ptD.y();
	tmp_pt = generate_random_point(tmp_quadix);
	
	Point2d ret_point(tmp_pt.x, tmp_pt.y);
	return ret_point;
}

///
/// Point2d
///
Point2d::Point2d(float x, float y)
	: point_(new wykobi::point2d<float>())
{
	point_->x = x;
	point_->y = y;
}

Point2d::Point2d(const Point2d& rhs) 
	: point_(new wykobi::point2d<float>())
{
	point_->x = rhs.x();
	point_->y = rhs.y();
}

Point2d& Point2d::operator=(const Point2d& pt)
{
	if (&pt == this) 
		return *this;

	point_->x = pt.x();
	point_->y = pt.y();
	return *this;
}

Point2d::~Point2d()
{
	delete point_;
	point_ = NULL;
}

float Point2d::x() const
{
	return point_->x;
}

float Point2d::y() const
{
	return point_->y;
}

///
/// Rectangle
///
Rectangle::Rectangle()
	: rectangle_(new wykobi::rectangle<float>())
{
}

Rectangle::Rectangle(const Point2d& pt1, const Point2d& pt2)
	: rectangle_(new wykobi::rectangle<float>())
{
	(*rectangle_)[0].x = pt1.x();
	(*rectangle_)[0].y = pt1.y();
	(*rectangle_)[1].x = pt2.x();
	(*rectangle_)[1].y = pt2.y();
}

Rectangle::Rectangle(const Rectangle& rhs)
	: rectangle_(new wykobi::rectangle<float>())
{
	(*rectangle_)[0] = (*(rhs.rectangle_))[0];
	(*rectangle_)[1] = (*(rhs.rectangle_))[1];
}

Rectangle::Rectangle(const wykobi::rectangle<float>& rect)
	: rectangle_(new wykobi::rectangle<float>())
{
	(*rectangle_)[0] = rect[0];
	(*rectangle_)[1] = rect[1];
}

Rectangle::~Rectangle()
{
	delete rectangle_;
	rectangle_ = NULL;
}

Rectangle& Rectangle::operator=(const Rectangle& rhs)
{
	if (&rhs == this)
		return *this;

	(*rectangle_)[0] = (*(rhs.rectangle_))[0];
	(*rectangle_)[1] = (*(rhs.rectangle_))[1];
	return *this;
}

bool Rectangle::point_in_rectangle(float x, float y) const
{
	return wykobi::point_in_rectangle(x, y, *rectangle_);
}

bool Rectangle::point_in_rectangle(const Point2d& pt) const
{
	wykobi::point2d<float> point;
	point.x = pt.x();
	point.y = pt.y();
	return wykobi::point_in_rectangle(point, *rectangle_);
}


///
/// Polygon
///
Polygon::Polygon()
	: polygon_(new wykobi::polygon<float, 2>())
{
}

Polygon::~Polygon()
{
	delete polygon_;
	polygon_ = NULL;
}

Polygon::Polygon(const Polygon& rhs) 
	: polygon_(new wykobi::polygon<float, 2>())
{
	//std::copy(rhs.polygon_->begin(), rhs.polygon_->end(), polygon_->begin());
	*polygon_ = *(rhs.polygon_);
} 

const Polygon& Polygon::operator=(const Polygon& rhs) 
{
	if (&rhs == this)
		return *this;

	//std::copy(rhs.polygon_->begin(), rhs.polygon_->end(), polygon_->begin());
	*polygon_ = *(rhs.polygon_);
	return *this;
}

bool Polygon::is_valid() const
{
	for (size_t i = 0; i < polygon_->size(); ++i)
	{
		wykobi::point2d<float> point1 = (*polygon_)[i];
		wykobi::point2d<float> point2 = (*polygon_)[(i+1)%polygon_->size()];
		wykobi::segment<float, 2> segment1;
		segment1[0] = point1;
		segment1[1] = point2;

		int num_of_intersection = 0;
		for (size_t j = 0; j < polygon_->size(); ++j)
		{
			if (i == j) continue;
			wykobi::point2d<float> point3 = (*polygon_)[j];
			wykobi::point2d<float> point4 = (*polygon_)[(j+1)%polygon_->size()];
			wykobi::segment<float, 2> segment2;
			segment2[0] = point3;
			segment2[1] = point4;
			if (wykobi::intersect(segment1, segment2))
			{
				++num_of_intersection;
			}
		}

		if (num_of_intersection != 2)
			return false;
	}

	return true;
}

bool Polygon::is_convex_polygon() const
{
	return wykobi::is_convex_polygon(*polygon_);
}

bool Polygon::point_in_convex_polygon(float x, float y) const
{
	wykobi::point2d<float> tmp_point;
	tmp_point.x = x;
	tmp_point.y = y;
	return wykobi::point_in_convex_polygon(tmp_point, *polygon_);
}

bool Polygon::point_in_convex_polygon(const Point2d& point) const
{
	return point_in_convex_polygon(point.x(), point.y());
}

bool Polygon::point_in_polygon(float x, float y) const
{
	wykobi::point2d<float> tmp_point;
	tmp_point.x = x;
	tmp_point.y = y;
	return wykobi::point_in_polygon(tmp_point, *polygon_);
}

bool Polygon::point_in_polygon(const Point2d& point) const
{
	return point_in_polygon(point.x(), point.y());
}

bool Polygon::point_on_polygon_edge(float x, float y) const
{
	return wykobi::point_on_polygon_edge(x, y, *polygon_);
}

void Polygon::centroid(float& x, float& y) const
{
	wykobi::centroid(*polygon_, x, y);
}

void Polygon::push_back(const Point2d& value)
{
	wykobi::point2d<float> point;
	point.x = value.x();
	point.y = value.y();
	polygon_->push_back(point);
}

Rectangle Polygon::aabb() const
{
	wykobi::rectangle<float> rect = wykobi::aabb(*polygon_);
	Rectangle ret_rect(rect);
	return ret_rect;
}

const Point2d Polygon::get_point(int index) const
{ 
	Point2d ret_pt((*polygon_)[index].x, (*polygon_)[index].y);
	return ret_pt;
}

size_t Polygon::size() const
{
	return polygon_->size();
}

} // namespace CGLib
