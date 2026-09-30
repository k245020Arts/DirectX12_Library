#include "Scene.h"
#include "../Pixel/Triangle/Triangle.h"
#include "../Pixel/Box/Box2D.h"
#include "../Pixel/CIcle/Cicle2D.h"

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
}

Scene::~Scene()
{
	delete test;
	delete test2;
	delete boxTest;
	delete cicleTest;
}

void Scene::Update()
{
	test->Update();
	test2->Update();
	boxTest->Update();
	cicleTest->Update();
}

void Scene::Draw()
{
	test->Draw();
	test2->Draw();
	boxTest->Draw();
	cicleTest->Draw();
}
