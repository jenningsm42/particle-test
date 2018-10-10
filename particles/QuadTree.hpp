#ifndef QUADTREE_HPP
#define QUADTREE_HPP
#include <memory>
#include <vector>

class Point {
public:
	Point() : x(0.f), y(0.f), vx(0.f), vy(0.f), m(1.f), ax(0.f), ay(0.f) {}
	Point(float x, float y, float vx, float vy, float m) : x(x), y(y), vx(vx), vy(vy), m(m), ax(0.f), ay(0.f) {}
	Point(const Point& p) : x(p.x), y(p.y), vx(p.vx), vy(p.vy), m(p.m), ax(p.ax), ay(p.ay) {}

	void update(float deltaTime) noexcept {
		// Maximum speed before linear cutoff - velocity will increase slower then
		const float maxSpeed = 200.f;

		vx += ax * deltaTime;
		vy += ay * deltaTime;

		if (vx > maxSpeed) vx = sqrt(maxSpeed * 4.f * vx) - maxSpeed;
		if (vx < -maxSpeed) vx = -(sqrt(maxSpeed * 4.f * -vx) - maxSpeed);
		if (vy > maxSpeed) vy = sqrt(maxSpeed * 4.f * vy) - maxSpeed;
		if (vy < -maxSpeed) vy = -(sqrt(maxSpeed * 4.f * -vy) - maxSpeed);

		x += vx * deltaTime;
		y += vy * deltaTime;
	}

	float x, y, vx, vy, ax, ay, m;
};

class BoundingShape {
public:
	virtual bool contains(const Point&) const noexcept = 0;
};

class Rectangle : public BoundingShape {
public:
	Rectangle(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
	Rectangle(const Rectangle& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}

	bool contains(const Point& p) const noexcept override {
		return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
	}

	float x, y, w, h;
};

class Circle : public BoundingShape {
public:
	Circle(float x, float y, float r) : x(x), y(y), r2(r*r) {}
	Circle(const Circle& c) : x(c.x), y(c.y), r2(c.r2) {}

	bool contains(const Point& p) const noexcept override {
		float dx = p.x - x;
		float dy = p.y - y;
		return dx * dx + dy * dy <= r2;
	}

	float x, y, r2;
};

class QuadTree {
public:
	QuadTree(const Rectangle&, size_t capacity);
	QuadTree(const QuadTree&);

	bool insert(Point*) noexcept;
	std::vector<Point*> query(const BoundingShape&) const noexcept;
	
	// Recalculate nodes after position update
	void update() noexcept;

	// To visualize the quadtree
	std::vector<Rectangle> getRectangles() const noexcept;

	size_t count() const noexcept;

	QuadTree& operator= (const QuadTree&);

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