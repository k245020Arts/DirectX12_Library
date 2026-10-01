#pragma once
#include <vector>

class Triangle;
class Box2D;
class Cicle2D;
class Pixel;
class Line2D;
class Star2D;

class Scene
{
public:
	
	Scene();
	~Scene();
	void Update(); // XVˆ—
	void Draw(); // •`‰æˆ—
private:

	Triangle* test;
	Triangle* test2;

	Box2D* boxTest;
	Cicle2D* cicleTest;

	std::vector<Pixel*> pixels;

	Line2D* line2D;
	Star2D* star2D;

	float value1;
	float value2;
};