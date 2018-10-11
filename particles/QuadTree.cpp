#include "QuadTree.hpp"
#include <algorithm>
#include <iostream>

QuadTree::QuadTree(const Rectangle& boundary, size_t capacity) : m_boundary(boundary), m_capacity(capacity), m_subdivided(false) {
}

bool QuadTree::insert(Point* p) noexcept {
    const auto& point = *p;
	if (!m_boundary.contains(point)) {
		return false;
	}

	if (m_points.size() < m_capacity) {
		m_points.push_back(p);
		return true;
	}

	if (!m_subdivided) {
		subdivide();
	}

    if (m_northWest->m_boundary.contains(point)) {
        return m_northWest->insert(p);
    }
    else if (m_northEast->m_boundary.contains(point)) {
        return m_northEast->insert(p);
    }
    else if (m_southWest->m_boundary.contains(point)) {
        return m_southWest->insert(p);
    } else {
        return m_southEast->insert(p);
    }
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
    rectangles.push_back(m_boundary);
	getRectanglesRecursive(rectangles);
	return rectangles;
}

void QuadTree::subdivide() noexcept {
	auto half_w = m_boundary.w / 2;
	auto half_h = m_boundary.h / 2;
    auto remainder_w = m_boundary.w % 2;
    auto remainder_h = m_boundary.h % 2;

	auto nw = Rectangle(m_boundary.x, m_boundary.y, half_w + remainder_w, half_h + remainder_h);
	auto ne = Rectangle(m_boundary.x + half_w + 1, m_boundary.y, half_w, half_h + remainder_h);
	auto sw = Rectangle(m_boundary.x, m_boundary.y + half_h + 1, half_w + remainder_w, half_h);
	auto se = Rectangle(m_boundary.x + half_w + 1, m_boundary.y + half_h + 1, half_w, half_h);

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
        if (range.bb.overlaps(m_northWest->m_boundary.bb)) {
            m_northWest->queryRecursive(range, found);
        }
        if (range.bb.overlaps(m_northEast->m_boundary.bb)) {
            m_northEast->queryRecursive(range, found);
        }
        if (range.bb.overlaps(m_southWest->m_boundary.bb)) {
            m_southWest->queryRecursive(range, found);
        }
        if (range.bb.overlaps(m_southEast->m_boundary.bb)) {
            m_southEast->queryRecursive(range, found);
        }
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

	return m_points.size() + m_northWest->count() + m_northEast->count() + m_southWest->count() + m_southEast->count();
}
