#pragma once

class Triangle;
class Box2D;
class Cicle2D;

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
};