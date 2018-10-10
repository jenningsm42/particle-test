#include <iostream>
#include <random>
#include <unordered_set>
#include <SFML/Graphics.hpp>
#include "QuadTree.hpp"

struct State {
	bool updating = false;
	bool showQuadtree = true;
};

int main() {
	std::random_device rd;
	auto generator = std::mt19937(rd());

	const unsigned int width = 1024;
	const unsigned int height = 768;
	const size_t pointCount = 100;
	const float radius = 5.f;
	const float mass = 20.f;
	const float initialMaxSpeed = 150.f;
	const float accelerationFactor = 50.f / mass;

	std::uniform_real_distribution<float> widthDistribution(radius, static_cast<float>(width) - radius);
	std::uniform_real_distribution<float> heightDistribution(radius, static_cast<float>(height) - radius);
	std::uniform_real_distribution<float> velocityDistribution(-initialMaxSpeed, initialMaxSpeed);

	auto boundary = Rectangle(0.f, 0.f, static_cast<float>(width), static_cast<float>(height));

	//auto qtree = QuadTree(boundary, 4);

	Point points[pointCount];
	for (int i = 0; i < pointCount; i++) {
		points[i] = Point(
			widthDistribution(generator),
			heightDistribution(generator),
			velocityDistribution(generator),
			velocityDistribution(generator),
			mass);
		//qtree.insert(&points[i]);
	}

	sf::RenderWindow window(sf::VideoMode(width, height), "Particles");

	sf::CircleShape particle(radius);
	particle.setFillColor(sf::Color::White);
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
			mx = static_cast<float>(m.x);
			my = static_cast<float>(m.y);
			mouseClicked = true;
		}

		float deltaTime = clock.restart().asSeconds();
		window.setTitle("Particles | fps: " + std::to_string(1.f / deltaTime));

		QuadTree qtree(boundary, 4);
		for (auto& p : points) {
			qtree.insert(&p);
		}

		if (state.updating) {
			// Particle collision detection
			for (auto& p : points) {
				Circle range(p.x, p.y, 2.f * radius);
				auto collidingPoints = qtree.query(range);
				for (auto& c : collidingPoints) {
					if (&p == c) continue;

					float dx = c->x - p.x;
					float dy = c->y - p.y;
					float d = std::max(std::min(sqrtf(dx*dx + dy*dy), 2.f * radius), radius);
					float nx = dx / d;
					float ny = dy / d;
					float pval = 2.f * (p.vx * nx + p.vy * ny - c->vx * nx + c->vy * ny) / (p.m + c->m);

					p.vx = p.vx - pval * p.m * nx;
					p.vy = p.vy - pval * p.m * ny;

					c->vx = c->vx + pval * c->m * nx;
					c->vy = c->vy + pval * c->m * ny;

					p.update(2.f * deltaTime);
					c->update(2.f * deltaTime);
				}
			}

			// Particle movement update
			for (auto& p : points) {
				if (mouseClicked) {
					p.ax = (mx - p.x) * accelerationFactor;
					p.ay = (my - p.y) * accelerationFactor;
				}
				else {
					p.ax = 0.f;
					p.ay = 0.f;
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

			std::cout << qtree.count() << std::endl;
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