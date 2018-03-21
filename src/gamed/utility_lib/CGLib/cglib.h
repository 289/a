#ifndef GAMED_UTILITY_LIB_CGLIB_H_
#define GAMED_UTILITY_LIB_CGLIB_H_

#include <cstddef>


namespace wykobi 
{
	template<typename T>
	class point2d;

	template<typename T>
	class rectangle;

	template<typename T, std::size_t Dimension>
	class polygon;
};


namespace CGLib {

// tag class
class copyable { };

/**
 * @brief Point2d
 */
class Point2d : copyable
{
public:
	Point2d(float x, float y);
	Point2d(const Point2d& rhs);
	~Point2d();

	Point2d& operator=(const Point2d& pt);

	float x() const;
	float y() const;

private:
	wykobi::point2d<float>* point_;
};

/**
 * @brief Rectangle
 */
class Rectangle : copyable
{
public:
	Rectangle();
	Rectangle(const Point2d& pt1, const Point2d& pt2);
	Rectangle(const Rectangle& rhs);
	Rectangle(const wykobi::rectangle<float>& rect);
	~Rectangle();

	Rectangle& operator=(const Rectangle& rhs);

	bool point_in_rectangle(float x, float y) const;
	bool point_in_rectangle(const Point2d& pt) const;

private:
	wykobi::rectangle<float>* rectangle_;
};

/**
 * @brief Polygon
 */
class Polygon : copyable
{
public:
	Polygon();
	~Polygon();

	Polygon(const Polygon&);
	const Polygon& operator=(const Polygon&);

	void push_back(const Point2d& value);

	size_t    size() const;
	bool      is_valid() const;
	bool      is_convex_polygon() const;
	bool      point_in_convex_polygon(float x, float y) const;
	bool      point_in_polygon(float x, float y) const;
	bool      point_in_convex_polygon(const Point2d& point) const;
	bool      point_in_polygon(const Point2d& point) const;
	bool      point_on_polygon_edge(float x, float y) const;
	void      centroid(float& x, float& y) const;

	Rectangle     aabb() const;
	const Point2d get_point(int index) const;


private:
	wykobi::polygon<float, 2>* polygon_;
};

// custom random function
typedef int (*custom_random)(int lower, int upper);

// init
void init_random_func(const custom_random func);

// gen random point
Point2d generate_random_point(Point2d ptA, Point2d ptB, Point2d ptC);
Point2d generate_random_point(Point2d ptA, Point2d ptB, Point2d ptC, Point2d ptD);

} // namespace CGLib

#endif // GAMED_UTILITY_LIB_CGLIB_H_
