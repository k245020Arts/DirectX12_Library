#include "Scene.h"
#include "../Primitives/Triangle/Triangle.h"
#include "../Primitives/Box/Box2D.h"
#include "../Primitives/CIcle/Cicle2D.h"
#include "../Primitives/Pixel/Pixel.h"
#include "../Primitives/Line/Line2D.h"
#include "../Primitives/Star/Star2D.h"
#include "../Texture/Texture2D.h"
#include "../Time/DeltaTime.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"
#include "../FBXModel/FBXModel.h"
#include "../Primitives/Box/Box3D.h"
#include"../Primitives/CIcle/Sphere3D.h"
#include "../Primitives/Line/Line3D.h"
#include "../Primitives/Triangle/Triangle3D.h"
#include "../Primitives/Triangle/TriangularPrism.h"
#include "../Primitives/Cylinder/Cylinder.h"
#include "../Primitives/Capsule/Capsule.h"
#include "../Camera3D/Camera3D.h"
#include "../Billboard/Billboard.h"
#include "../Input/InputManager.h"


#define ORIGINAL_MOUSE_POINTER
//#define ON2D

static const float CLICK_TIME = 0.2f;

Scene::Scene()
{
#ifdef ON2D
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
	line2D->SetColor(Vector4(1.0f, 0.0f, 1.0f, 1.0f));

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

	mousePointer = new Texture2D();
	mousePointer->Load("data/textest.png");
	mousePointer->GetTransform().position = Vector3(200, 200, 0);
	//texture2D2->SetSize(256, 256);
	mousePointer->SetRect(Vector4(0.0f, 0.0f, 128.0f, 128.0f));
	//texture2D2->SetFlipX(false);
	//texture2D2->SetFlipY(true);

	texture2D3 = new Texture2D();
	texture2D3->Load("data/animation2.png");
	texture2D3->GetTransform().position = Vector3(400, 400, 0);
	texture2D3->SetRect(Vector4(0.0f, 0.0f, 240.0f, 240.0f));
	texture2D3->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
#endif // ON2D

	InputManager::GetInstance()->SetMouseDraw(false);
	//L"data/Alicia/FBX/Alicia_solid_Unity.FBX"
	/*model = new FBXModel(); 
	model->Load("data/Alicia/FBX/Alicia_solid_Unity.FBX");*/

#ifdef ORIGINAL_MOUSE_POINTER
	clickTime = 0.0f;
	mousePointer = new Texture2D();
	mousePointer->GetTransform().scale = Vector3(0.4f, 0.4f, 0.4f);
	mousePointer->Load("data/mousePointer.png");
	mousePointer->GetTransform().position = Vector3(200, 200, 0);
	mousePointer->SetRect(Vector4(0.0f, 0.0f, 96.2f, 120.3f));

#endif
	sphere3D = new Sphere3D();
	sphere3D->GetTransform().position = Vector3(0.0f,0.0f,0.0f);
	sphere3D->SetSphereParam(Sphere3D::SphereParam(50.0f));
	sphere3D->SetFill(true);

	box3D = new Box3D();
	box3D->GetTransform().position = Vector3(-100.0f, 0.0f, 0.0f);
	box3D->SetLength(Vector3(200.0f, 200.0f, 50.0f));
	box3D->SetFill(false);

	line3D = new Line3D();
	line3D->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);
	line3D->SetStartAndEnd(Vector3(100, 100, 500),Vector3(200,200,-500));

	triangle3D = new Triangle3D();

	triangle3D->GetTransform().position = Vector3(150.0f, 0.0f, 0.0f);
	triangle3D->SetLength(Vector3(120.0f, 200.0f, 150.0f));
	triangle3D->SetFill(false);

	cylinder = new Cylinder();

	cylinder->GetTransform().position = Vector3(-170.0f, 0.0f, 150.0f);
	cylinder->SetCylinderParam(Cylinder::CylinderParam(50.0f, 200.0f));
	cylinder->SetFill(false);

	capsule = new Capsule();

	capsule->GetTransform().position = Vector3(170.0f, 0.0f, 150.0f);
	capsule->SetCapsuleParam(Capsule::CapsuleParam(20, 10, 20, 100));
	capsule->SetColor(Vector4(1.0f,0.0f,0.0f,1.0f));

	capsule->SetAlpha(0.01f);
	capsule->SetBlendMode(BlendState::NO_BLEND);
	capsule->SetFill(true);

	billboard = new Billboard();
	billboard->Load("data/textest.png");
	billboard->GetTransform().position = Vector3(170.0f, 50.0f, 250.0f);
	billboard->SetRectUV(Vector4(0.0f,0.0f,0.5f,0.5f));
	//billboard->SetBillBoardType(true);
}

Scene::~Scene()
{
	/*delete model;*/
	/*delete box3D;
	delete sphere3D;
	delete line3D;
	delete triangle3D;
	delete cylinder;
	delete capsule;
	delete billboard;*/
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

//#define CAMERA_TEST

void Scene::Update()
{
#ifdef  ON2D
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
#endif //  ON2D

	/*model->GetTransform().rotation.y += 0.005f;
	model->Update();*/

	//box3D->Update();
	box3D->GetTransform().rotation.y += 0.01f;

	box3D->GetTransform().position.z += InputManager::GetInstance()->GetMouseWheelDelta();


	//sphere3D->Update();
	sphere3D->GetTransform().rotation.x += 0.01f;
	
	//line3D->Update();

	//triangle3D->Update();
	if (InputManager::GetInstance()->GetKeyInput(KeyCode::W)) {
		triangle3D->GetTransform().rotation.y += 0.01f;
	}
	

	//cylinder->Update();
	cylinder->GetTransform().rotation.y += 0.01f;

	//capsule->Update();
	capsule->GetTransform().rotation.y += 0.01f;

	billboard->GetTransform().rotation.y -= 0.01f;
	/*Camera3D::GetInstance()->SetTarget(billboard->GetTransform().position);
	Vector3 nowpos = Camera3D::GetInstance()->GetPosition();
	Vector3 finalPos = Vector3(nowpos.x + -5.0f, nowpos.y, nowpos.z - 1.0f);
	Camera3D::GetInstance()->SetPosition(finalPos);*/

#ifdef CAMERA_TEST

	auto* camera = Camera3D::GetInstance();

	// ƒrƒ‹ƒ{[ƒh‚ð’Ž‹‚·‚é
	Vector3 target = billboard->GetTransform().position;
	camera->SetTarget(target);

	// ‰ñ“]‘¬“xi1•b‚Å1Žüj
	constexpr float ROTATION_SPEED =
		DirectX::XM_2PI / 6.0f; // 3•b‚Å1Žü

	// Œo‰ßŽžŠÔ‚ð“n‚¹‚é‚È‚ç deltaTime ‚ðŽg‚¤
	cameraAngle += ROTATION_SPEED * DeltaTime::GetInstance()->GetDeltaTime();

	// 1Žü‚µ‚½‚çŠp“x‚ð–ß‚·
	if (cameraAngle >= DirectX::XM_2PI)
	{
		cameraAngle -= DirectX::XM_2PI;
	}

	// ’Ž‹“_‚©‚ç‚Ì‹——£
	constexpr float RADIUS = 100.0f;
	constexpr float HEIGHT = 50.0f;

	const Vector3 nowPos = { 0.0f, 150.0f, -200.0f };

	// ‰~‹O“¹ã‚ÌƒJƒƒ‰ˆÊ’ui¶ŽèÀ•WŒn‚Å‚à‚±‚ÌˆÊ’uŒvŽZ‚ÅOKj
	Vector3 finalPos(
		nowPos.x + std::sin(cameraAngle) * RADIUS,
		nowPos.y + HEIGHT,
		nowPos.z - std::cos(cameraAngle) * RADIUS
	);
	 //std::sin(cameraAngle) * RADIUS,
	camera->SetPosition(finalPos);

#endif // CAMERA_TEST
#ifdef ORIGINAL_MOUSE_POINTER

	Vector2 mousePos = InputManager::GetInstance()->GetMousePosition();

	mousePointer->GetTransform().position.x = mousePos.x;
	mousePointer->GetTransform().position.y = mousePos.y;

	if (clickTime > 0.0f) {
		clickTime -= DeltaTime::GetInstance()->GetDeltaTime();
		mousePointer->SetRect(Vector4(0.0f, 120.3f, 96.2f, 120.3f));
	}
	else {
		mousePointer->SetRect(Vector4(0.0f, 0.0f, 96.2f, 120.3f));
		if (InputManager::GetInstance()->GetMouseInputDown(MouseCode::Left)){
			clickTime = CLICK_TIME;
		}
	}
#endif
}

void Scene::Draw()
{
	/*model->Draw();*/
	//box3D->Draw();
	//sphere3D->Draw();
	//line3D->Draw();

	//triangle3D->Draw();
	//cylinder->Draw();

	//capsule->Draw();
}
