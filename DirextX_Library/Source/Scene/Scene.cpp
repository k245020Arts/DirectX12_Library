#include "Scene.h"
#include "../Pixel/Triangle/Triangle.h"

Scene::Scene()
{
	test = new Triangle();
	test->GetTransform().position = Vector3(200.0f, 0.0f, 0.0f);

	test2 = new Triangle();
	test2->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);
}

Scene::~Scene()
{
	delete test;
	delete test2;
}

void Scene::Update()
{
	test->Update();
	test2->Update();
}

void Scene::Draw()
{
	test->Draw();
	test2->Draw();
}
