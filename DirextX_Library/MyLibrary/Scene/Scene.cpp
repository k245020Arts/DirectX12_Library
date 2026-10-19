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
#include <algorithm>


//#define ORIGINAL_MOUSE_POINTER
//#define ON2D
//#define ON3D

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

	
	//L"data/Alicia/FBX/Alicia_solid_Unity.FBX"
	model = new FBXModel(); 
	model->Load(L"data/Dragon 2.5_fbx.fbx",false,true);

#ifdef ORIGINAL_MOUSE_POINTER
	clickTime = 0.0f;
	mousePointer = new Texture2D();
	mousePointer->GetTransform().scale = Vector3(0.4f, 0.4f, 0.4f);
	mousePointer->Load("data/mousePointer.png");
	mousePointer->GetTransform().position = Vector3(200, 200, 0);
	mousePointer->SetRect(Vector4(0.0f, 0.0f, 96.2f, 120.3f));
	InputManager::GetInstance()->SetMouseDraw(false);
#endif

#ifdef ON3D
	sphere3D = new Sphere3D();
	sphere3D->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);
	sphere3D->SetSphereParam(Sphere3D::SphereParam(50.0f));
	sphere3D->SetFill(true);

	box3D = new Box3D();
	box3D->GetTransform().position = Vector3(-100.0f, 0.0f, 0.0f);
	box3D->SetLength(Vector3(200.0f, 200.0f, 50.0f));
	box3D->SetFill(false);

	line3D = new Line3D();
	line3D->GetTransform().position = Vector3(0.0f, 0.0f, 0.0f);
	line3D->SetStartAndEnd(Vector3(100, 100, 500), Vector3(200, 200, -500));

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
	capsule->SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

	capsule->SetAlpha(0.01f);
	capsule->SetBlendMode(BlendState::NO_BLEND);
	capsule->SetFill(true);

	billboard = new Billboard();
	billboard->Load("data/textest.png");
	billboard->GetTransform().position = Vector3(170.0f, 50.0f, 250.0f);
	billboard->SetRectUV(Vector4(0.0f, 0.0f, 0.5f, 0.5f));
	billboard->SetBillBoardType(false);
#endif // ON3D

	
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

	model->GetTransform().rotation.y += 0.005f;
	//model->Update();

	//box3D->Update();
	

	//box3D->GetTransform().position.z += InputManager::GetInstance()->GetMouseWheelDelta();
#ifdef ON3D
	box3D->GetTransform().rotation.y += 0.01f;

	

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
#endif

	MoveCamera();
#ifdef CAMERA_TEST

	auto* camera = Camera3D::GetInstance();

	// ビルボードを注視する
	Vector3 target = billboard->GetTransform().position;
	camera->SetTarget(target);

	// 回転速度（1秒で1周）
	constexpr float ROTATION_SPEED =
		DirectX::XM_2PI / 6.0f; // 3秒で1周

	// 経過時間を渡せるなら deltaTime を使う
	cameraAngle += ROTATION_SPEED * DeltaTime::GetInstance()->GetDeltaTime();

	// 1周したら角度を戻す
	if (cameraAngle >= DirectX::XM_2PI)
	{
		cameraAngle -= DirectX::XM_2PI;
	}

	// 注視点からの距離
	constexpr float RADIUS = 100.0f;
	constexpr float HEIGHT = 50.0f;

	const Vector3 nowPos = { 0.0f, 150.0f, -200.0f };

	// 円軌道上のカメラ位置（左手座標系でもこの位置計算でOK）
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

void Scene::MoveCamera()
{
	auto* camera = Camera3D::GetInstance();
	auto* input = InputManager::GetInstance();

	Vector3 camPos = camera->GetPosition();
	Vector3 target = camera->GetTarget();

	// カメラから注視点までの距離
	Vector3 toTarget = target - camPos;
	float distance = toTarget.Size();

	// 距離が0だと方向を計算できないため補正
	distance = (std::max)(distance, 0.1f);

	// マウスホイールでズーム
	float wheel = static_cast<float>(input->GetMouseWheelDelta());

	// WM_MOUSEWHEEL の値が120単位の場合
	wheel /= 60.0f;

	const float zoomSpeed = 10.0f;

	if (wheel != 0.0f)
	{
		Vector3 forward = toTarget.Normalize();

		// 注視点に向かって移動
		camPos += forward * (wheel * zoomSpeed);

		// 注視点を通り越さないように距離を制限
		Vector3 remaining = target - camPos;
		float newDistance = remaining.Size();

		if (newDistance < 0.1f)
		{
			camPos = target - forward * 0.1f;
		}

		distance = (target - camPos).Size();
	}

	// 右ボタン：注視点を中心に回転
	if (input->GetMouseInput(MouseCode::Right))
	{
		Vector2 mouseDelta =
			input->GetMouseDeltaPosition();

		const float rotateSpeed = 0.01f;

		cameraYaw += mouseDelta.x * rotateSpeed;
		cameraPitch -= mouseDelta.y * rotateSpeed;

		// 真上・真下での反転を防ぐ
		constexpr float LIMIT =
			DirectX::XM_PIDIV2 - 0.01f;

		cameraPitch = std::clamp(
			cameraPitch, -LIMIT, LIMIT
		);
	}

	// 最新のYaw・Pitchから前方向を再計算
	Vector3 forward;
	forward.x = std::sin(cameraYaw) * std::cos(cameraPitch);
	forward.y = std::sin(cameraPitch);
	forward.z = std::cos(cameraYaw) * std::cos(cameraPitch);

	forward = forward.Normalize();

	// 中ボタン：カメラと注視点を一緒に平行移動
	if (input->GetMouseInput(MouseCode::Middle))
	{
		Vector2 mouseDelta = input->GetMouseDeltaPosition();

		// カメラの右方向
		Vector3 right( std::cos(cameraYaw),0.0f,-std::sin(cameraYaw));

		// カメラの上方向
		Vector3 up(-std::sin(cameraYaw) * std::sin(cameraPitch),std::cos(cameraPitch),-std::cos(cameraYaw) * std::sin(cameraPitch));

		const float panSpeed = distance * 0.002f;

		Vector3 pan =right * (-mouseDelta.x * panSpeed) + up * (mouseDelta.y * panSpeed);

		camPos += pan;
		target += pan;
	}

	// 注視点を中心にカメラ位置を再計算
	// 回転中は注視点までの距離を維持する
	if (input->GetMouseInput(MouseCode::Right))
	{
		camPos = target - forward * distance;
	}

	camera->SetPosition(camPos);
	camera->SetTarget(target);
}