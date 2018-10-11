#ifndef QUADTREE_HPP
#define QUADTREE_HPP
#include <cmath>
#include <memory>
#include <vector>

#include <iostream>

class Point {
public:
	Point() : x(0.f), y(0.f), vx(0.f), vy(0.f), m(1.f), ax(0.f), ay(0.f) {}
	Point(double x, double y, double vx, double vy, double m) : x(x), y(y), vx(vx), vy(vy), m(m), ax(0.f), ay(0.f) {}
	Point(const Point& p) : x(p.x), y(p.y), vx(p.vx), vy(p.vy), m(p.m), ax(p.ax), ay(p.ay) {}

	void update(double deltaTime) noexcept {
		// Maximum speed before linear cutoff - velocity will increase slower then
		const double maxSpeed = 200.0;

		vx += ax * deltaTime;
		vy += ay * deltaTime;

		if (vx > maxSpeed) vx = sqrt(maxSpeed * 4.0 * vx) - maxSpeed;
		if (vx < -maxSpeed) vx = -(sqrt(maxSpeed * 4.0 * -vx) - maxSpeed);
		if (vy > maxSpeed) vy = sqrt(maxSpeed * 4.0 * vy) - maxSpeed;
		if (vy < -maxSpeed) vy = -(sqrt(maxSpeed * 4.0 * -vy) - maxSpeed);

		x += vx * deltaTime;
		y += vy * deltaTime;
	}

	double x, y, vx, vy, m, ax, ay;
};

class BoundingBox {
public:
    BoundingBox(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    BoundingBox(const BoundingBox& bb) : x(bb.x), y(bb.y), w(bb.w), h(bb.h) {}

    bool overlaps(const BoundingBox& bb) const noexcept {
        return !(bb.x > x + w || bb.x + bb.w < x || bb.y > y + h || bb.y + bb.h < y);
    }

    int x, y, w, h;
};

class BoundingShape {
public:
    BoundingShape(const BoundingBox& bb) : bb(bb) {}
	virtual bool contains(const Point&) const noexcept = 0;

    BoundingBox bb;
};

class Rectangle : public BoundingShape {
public:
	Rectangle(int x, int y, int w, int h) : BoundingShape(BoundingBox(x, y, w, h)), x(x), y(y), w(w), h(h) {}
	Rectangle(const Rectangle& r) : BoundingShape(r.bb), x(r.x), y(r.y), w(r.w), h(r.h) {}

	bool contains(const Point& p) const noexcept override {
        const int px = static_cast<int>(p.x);
        const int py = static_cast<int>(p.y);
		return px >= x && px <= x + w && py >= y && py <= y + h;
	}

	int x, y, w, h;
};

class Circle : public BoundingShape {
public:
	Circle(double x, double y, double r) : BoundingShape(BoundingBox(x-r, y-r, x+r, y+r)), x(x), y(y), r2(r*r) {}
	Circle(const Circle& c) : BoundingShape(c.bb), x(c.x), y(c.y), r2(c.r2) {}

	bool contains(const Point& p) const noexcept override {
		double dx = p.x - x;
		double dy = p.y - y;
		return dx * dx + dy * dy <= r2;
	}

	double x, y, r2;
};

class QuadTree {
public:
	QuadTree(const Rectangle&, size_t capacity);

	bool insert(Point*) noexcept;
	std::vector<Point*> query(const BoundingShape&) const noexcept;

	// Recalculate nodes after position update
	void update() noexcept;

	// To visualize the quadtree
	std::vector<Rectangle> getRectangles() const noexcept;

	size_t count() const noexcept;

private:
	Rectangle m_boundary;
	const size_t m_capacity;
	bool m_subdivided;
	std::vector<Point*> m_points;
	std::unique_ptr<QuadTree> m_northWest;
	std::unique_ptr<QuadTree> m_northEast;
	std::unique_ptr<QuadTree> m_southWest;
	std::unique_ptr<QuadTree> m_southEast;

	inline void subdivide() noexcept;
	inline void getRectanglesRecursive(std::vector<Rectangle>& found) const noexcept;
	inline void queryRecursive(const BoundingShape&, std::vector<Point*>& found) const noexcept;
	inline bool updateRecursive(std::vector<Point*>&) noexcept;
};

#endif
