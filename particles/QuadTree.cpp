#include "QuadTree.hpp"
#include <algorithm>
#include <iostream>

QuadTree::QuadTree(const Rectangle& boundary, size_t capacity) : m_boundary(boundary), m_capacity(capacity), m_subdivided(false) {
}

QuadTree::QuadTree(const QuadTree& other) : m_boundary(other.m_boundary), m_capacity(other.m_capacity), m_subdivided(other.m_subdivided) {
	std::copy(other.m_points.begin(), other.m_points.end(), m_points.begin());
	if (m_subdivided) {
		m_northWest = std::make_unique<QuadTree>(QuadTree(*other.m_northWest.get()));
		m_northEast = std::make_unique<QuadTree>(QuadTree(*other.m_northEast.get()));
		m_southWest = std::make_unique<QuadTree>(QuadTree(*other.m_southWest.get()));
		m_southEast = std::make_unique<QuadTree>(QuadTree(*other.m_southEast.get()));
	}
}

bool QuadTree::insert(Point* p) noexcept {
	if (!m_boundary.contains(*p)) {
		return false;
	}

	if (m_points.size() < m_capacity) {
		m_points.push_back(p);
		return true;
	}

	if (!m_subdivided) {
		subdivide();
	}

	return m_northWest->insert(p) ||
		m_northEast->insert(p) ||
		m_southWest->insert(p) ||
		m_southEast->insert(p);
}

std::vector<Point*> QuadTree::query(const BoundingShape& range) const noexcept {
	std::vector<Point*> points;
	queryRecursive(range, points);
	return points;
}

void QuadTree::update() noexcept {
	std::vector<Point*> removedPoints;
	updateRecursive(removedPoints);
	for (auto p : removedPoints) {
		auto res = insert(p);
		if (!res) std::cout << "could not insert " << p->x << ", " << p->y << std::endl;
	}
}

std::vector<Rectangle> QuadTree::getRectangles() const noexcept {
	std::vector<Rectangle> rectangles;
	getRectanglesRecursive(rectangles);
	return rectangles;
}

QuadTree& QuadTree::operator= (const QuadTree& other) {
	return QuadTree(other);
}

void QuadTree::subdivide() noexcept {
	auto half_w = m_boundary.w / 2.f;
	auto half_h = m_boundary.h / 2.f;

	auto nw = Rectangle(m_boundary.x, m_boundary.y, half_w, half_h);
	auto ne = Rectangle(m_boundary.x + half_w, m_boundary.y, half_w, half_h);
	auto sw = Rectangle(m_boundary.x, m_boundary.y + half_h, half_w, half_h);
	auto se = Rectangle(m_boundary.x + half_w, m_boundary.y + half_h, half_w, half_h);

	m_northWest = std::make_unique<QuadTree>(nw, m_capacity);
	m_northEast = std::make_unique<QuadTree>(ne, m_capacity);
	m_southWest = std::make_unique<QuadTree>(sw, m_capacity);
	m_southEast = std::make_unique<QuadTree>(se, m_capacity);

	m_subdivided = true;
}

void QuadTree::getRectanglesRecursive(std::vector<Rectangle>& found) const noexcept {
	found.push_back(m_boundary);

	if (m_subdivided) {
		m_northWest->getRectanglesRecursive(found);
		m_northEast->getRectanglesRecursive(found);
		m_southWest->getRectanglesRecursive(found);
		m_southEast->getRectanglesRecursive(found);
	}
}

void QuadTree::queryRecursive(const BoundingShape& range, std::vector<Point*>& found) const noexcept {
	for (auto p : m_points) {
		if (range.contains(*p)) {
			found.push_back(p);
		}
	}

	if (m_subdivided) {
		m_northWest->queryRecursive(range, found);
		m_northEast->queryRecursive(range, found);
		m_southWest->queryRecursive(range, found);
		m_southEast->queryRecursive(range, found);
	}
}

bool QuadTree::updateRecursive(std::vector<Point*>& removedPoints) noexcept {
	m_points.erase(std::remove_if(m_points.begin(), m_points.end(), [&](Point* p) {
		if (!m_boundary.contains(*p)) {
			removedPoints.push_back(p);
			return true;
		}
		return false;
	}), m_points.end());

	if (m_subdivided) {
		auto empty = m_northWest->updateRecursive(removedPoints) &&
			m_northEast->updateRecursive(removedPoints) &&
			m_southWest->updateRecursive(removedPoints) &&
			m_southEast->updateRecursive(removedPoints);

		if (empty) {
			m_subdivided = false;
		}
	}

	return m_points.size() == 0;
}

size_t QuadTree::count() const noexcept {
	if (!m_subdivided) {
		return m_points.size();
	}

	return m_northWest->count() + m_northEast->count() + m_southWest->count() + m_southEast->count() + m_points.size();
}