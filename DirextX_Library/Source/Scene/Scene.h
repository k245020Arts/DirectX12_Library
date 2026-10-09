#pragma once
#include <vector>

class Triangle;
class Box2D;
class Box3D;
class Cicle2D;
class Sphere3D;
class Pixel;
class Line2D;
class Line3D;
class Star2D;
class Texture2D;
class FBXModel;

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

	float alphaMode;

	Texture2D* texture2D;
	Texture2D* texture2D1;
	Texture2D* texture2D2;
	Texture2D* texture2D3;

	float animationCount;
	int count;

	FBXModel* model;

	Box3D* box3D;
	Sphere3D* sphere3D;
	Line3D* line3D;
};