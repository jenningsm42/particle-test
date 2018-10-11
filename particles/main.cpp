#include <iostream>
#include <random>
#include <unordered_set>
#include <SFML/Graphics.hpp>
#include "QuadTree.hpp"

struct State {
	bool updating = true;
	bool showQuadtree = false;
};

int main() {
	std::random_device rd;
	auto generator = std::mt19937(rd());

	sf::RenderWindow window(sf::VideoMode(1024, 768), "Particles");

    // To handle i3 automatically resizing window
    auto windowSize = window.getSize();
    const unsigned int width = windowSize.x;
    const unsigned int height = windowSize.y;

	const size_t pointCount = 1000;
	const double radius = 0.5;
	const double mass = 40.0;
	const double initialMaxSpeed = 10.0;
	const double accelerationFactor = 10.0 / mass;

	std::uniform_real_distribution<double> widthDistribution(radius, static_cast<double>(width) - radius);
	std::uniform_real_distribution<double> heightDistribution(radius, static_cast<double>(height) - radius);
	std::uniform_real_distribution<double> velocityDistribution(-initialMaxSpeed, initialMaxSpeed);

	auto boundary = Rectangle(0, 0, width, height);

	//auto qtree = QuadTree(boundary, 1);

	Point points[pointCount];
	for (size_t i = 0; i < pointCount; i++) {
		points[i] = Point(
			widthDistribution(generator),
			heightDistribution(generator),
			velocityDistribution(generator),
			velocityDistribution(generator),
			mass);
		//qtree.insert(&points[i]);
	}

	sf::CircleShape particle(std::max(1.f, static_cast<float>(radius)));
	particle.setFillColor(sf::Color::Blue);
	particle.setOrigin(radius, radius);

	sf::RectangleShape rectangle;
	rectangle.setOutlineThickness(1.f);
	rectangle.setFillColor(sf::Color::Transparent);
	rectangle.setOutlineColor(sf::Color::White);

	State state;

	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::Space: state.updating = !state.updating; break;
				case sf::Keyboard::Q: state.showQuadtree = !state.showQuadtree; break;
				default: break;
				}
			}
		}

		bool mouseClicked = false;
		float mx, my;

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			auto m = sf::Mouse::getPosition(window);
			mx = static_cast<double>(m.x);
			my = static_cast<double>(m.y);
			mouseClicked = true;
		}

		double deltaTime = clock.restart().asSeconds();
		window.setTitle("Particles | fps: " + std::to_string(1.f / deltaTime));

		QuadTree qtree(boundary, 4);
		for (auto& p : points) {
			qtree.insert(&p);
		}

		if (state.updating) {
			// Particle collision detection
            std::unordered_set<Point*> visited;
			for (auto& p : points) {
				Circle range(p.x, p.y, 2.0 * radius);

				auto collidingPoints = qtree.query(range);
				for (auto& c : collidingPoints) {
					if (&p == c) continue;
                    if (visited.count(c) > 0) continue;

                    visited.insert(c);

					double dx = c->x - p.x;
					double dy = c->y - p.y;
                    double d2 = std::min(std::max(radius, dx * dx + dy * dy), 2.0 * radius);
					double d = sqrt(d2);
					double nx = dx / d;
					double ny = dy / d;
					double pval = 2.0 * (p.vx * nx + p.vy * ny - c->vx * nx + c->vy * ny) / (p.m + c->m);

					p.vx = p.vx - pval * p.m * nx;
					p.vy = p.vy - pval * p.m * ny;

					c->vx = c->vx + pval * c->m * nx;
					c->vy = c->vy + pval * c->m * ny;

					c->update(2.0 * deltaTime);
				}

                p.update(2.0 * deltaTime);
			}

			// Particle movement update
			for (auto& p : points) {
				if (mouseClicked) {
					p.ax = (mx - p.x) * accelerationFactor;
					p.ay = (my - p.y) * accelerationFactor;
				}
				else {
					p.ax = 0.0;
					p.ay = 0.0;
				}

				p.update(deltaTime);

				if (p.x < radius || p.x > width - radius) {
					p.vx = -p.vx;
					p.x = (p.x < radius) ? radius : width - radius;
				}

				if (p.y < radius || p.y > height - radius) {
					p.vy = -p.vy;
					p.y = (p.y < radius) ? radius : height - radius;
				}
			}

			//qtree.update();
		}

		window.clear();

		if (state.showQuadtree) {
			for (const auto& r : qtree.getRectangles()) {
				rectangle.setPosition(r.x, r.y);
				rectangle.setSize(sf::Vector2f(r.w, r.h));
				window.draw(rectangle);
			}
		}

		for (const auto& p : points) {
			particle.setPosition(p.x, p.y);
			window.draw(particle);
		}

		window.display();
	}

	return 0;
}
