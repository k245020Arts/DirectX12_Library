#include "Scene.h"
#include "../Pixel/Triangle/Triangle.h"
#include "../Pixel/Box/Box2D.h"
#include "../Pixel/CIcle/Cicle2D.h"
#include "../Pixel/Pixel/Pixel.h"
#include "../Pixel/Line/Line2D.h"
#include "../Pixel/Star/Star2D.h"

Scene::Scene()
{
	test = new Triangle();
	test->GetTransform().position = Vector3(200.0f, 0.0f, 0.0f);

	test2 = new Triangle();
	test2->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);

	boxTest = new Box2D();
	boxTest->GetTransform().position = Vector3(400.0f, 0.0f, 0.0f);

	cicleTest = new Cicle2D();
	cicleTest->GetTransform().position = Vector3(600.0f, 0.0f, 0.0f);

	for (int i = 0; i < 5; i++) {
		Pixel* pixel = new Pixel();
		pixel->GetTransform().position = Vector3(i, 200.0f, 0.0f);
		pixels.emplace_back(pixel);
	}

	line2D = new Line2D();
	line2D->GetTransform().position = Vector3(0, 200, 0);

	star2D = new Star2D();
	star2D->GetTransform().position = Vector3(100, 100, 0);
}

Scene::~Scene()
{
	delete test;
	delete test2;
	delete boxTest;
	delete cicleTest;

	for (int i = 0; i < 5; i++) {
		delete pixels[i];
	}
	pixels.clear();

	delete line2D;
	delete star2D;
}

void Scene::Update()
{
	test->Update();
	test2->Update();
	boxTest->Update();
	cicleTest->Update();

	for (auto pixel : pixels) {
		pixel->Update();
	}

	line2D->Update();
	star2D->Update();
}

void Scene::Draw()
{
	test->Draw();
	test2->Draw();
	boxTest->Draw();
	cicleTest->Draw();

	for (auto pixel: pixels) {
		pixel->Draw();
	}

	line2D->Draw();
	star2D->Draw();
}
