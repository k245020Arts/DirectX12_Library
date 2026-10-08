#include "Scene.h"
#include "../Pixel/Triangle/Triangle.h"
#include "../Pixel/Box/Box2D.h"
#include "../Pixel/CIcle/Cicle2D.h"
#include "../Pixel/Pixel/Pixel.h"
#include "../Pixel/Line/Line2D.h"
#include "../Pixel/Star/Star2D.h"
#include "../Texture/Texture2D.h"
#include "../Time/DeltaTime.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"

Scene::Scene()
{
	test = new Triangle();
	test->GetTransform().position = Vector3(200.0f, 0.0f, 0.0f);

	test2 = new Triangle();
	test2->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);

	boxTest = new Box2D();
	boxTest->GetTransform().position = Vector3(400.0f, 0.0f, 0.0f);

	cicleTest = new Cicle2D();
	cicleTest->GetTransform().position = Vector3(600.0f, 200.0f, 0.0f);

	for (int i = 0; i < 5; i++) {
		Pixel* pixel = new Pixel();
		pixel->GetTransform().position = Vector3((float)i, 200.0f, 0.0f);
		pixels.emplace_back(pixel);
	}

	line2D = new Line2D();
	line2D->GetTransform().position = Vector3(0, 200, 0);

	star2D = new Star2D();
	star2D->GetTransform().position = Vector3(200, 100, 0);
	star2D->SetFill(true);
	cicleTest->SetBlendMode(BlendState::NO_BLEND);
	line2D->SetColor(Vector4(1.0f,0.0f,1.0f,1.0f));

	/*Star2D* star2D2 = new Star2D();
	star2D->GetTransform().position = Vector3(350, 200, 0);
	star2D2->SetDrawOrder(10);*/

	/*texture2D = new Texture2D();
	texture2D->Load("data/f002.png");
	texture2D->GetTransform().position = Vector3(500, 200, 0);

	texture2D1 = new Texture2D();
	texture2D1->Load("data/f002.png");
	texture2D1->GetTransform().position = Vector3(700, 200, 0);

	texture2D->SetBlendMode(BlendState::ALPHA);
	texture2D->SetAlpha(0.01f);*/

	texture2D2 = new Texture2D();
	texture2D2->Load("data/textest.png");
	texture2D2->GetTransform().position = Vector3(200, 200, 0);
	//texture2D2->SetSize(256, 256);
	texture2D2->SetRect(Vector4(0.0f, 0.0f, 128.0f, 128.0f));
	//texture2D2->SetFlipX(false);
	//texture2D2->SetFlipY(true);

	texture2D3 = new Texture2D();
	texture2D3->Load("data/animation2.png");
	texture2D3->GetTransform().position = Vector3(400, 400, 0);
	texture2D3->SetRect(Vector4(0.0f, 0.0f, 240.0f, 240.0f));
	texture2D3->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
}

Scene::~Scene()
{
	/*delete test;
	delete test2;
	delete boxTest;
	delete cicleTest;

	for (int i = 0; i < 5; i++) {
		delete pixels[i];
	}
	pixels.clear();

	delete line2D;
	delete star2D;*/
}

void Scene::Update()
{
	value1 = 50;
	value2 = 100;
	cicleTest->SetRadius(value1);

	alphaMode += 0.001f;
	cicleTest->SetAlpha(alphaMode);

	//texture2D2->GetTransform().position.y += 1.0f;

	const int animationSize = 240;

	animationCount += DeltaTime::GetInstance()->GetDeltaTimeMulTimeScale();
	if (animationCount >= 0.2f) {
		texture2D3->SetRect(Vector4(count * (float)animationSize, 0.0f, animationSize, animationSize));
		animationCount = 0.0f;
		count++;
		if (count >= 10) {
			count = 0;
		}
	}
	
	/*texture2D1->GetTransform().rotation.z += 0.02f;
	texture2D->GetTransform().rotation.z += 0.01f;*/
}

void Scene::Draw()
{
	
}
