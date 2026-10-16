#include "Camera3D.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"

Camera3D::Camera3D()
{
	position = { 0.0f, 150.0f, -300.0f };
	target = { 0.0f, 0.0f, 0.0f };
	upVec = { 0.0f, 1.0f, 0.0f };

	fovAngle = 60.0f;
	nearFar.cameraToNear = 5.0f;
	nearFar.cameraToFar = 10000.0f;
}

Camera3D::~Camera3D()
{

}

void Camera3D::Update()
{
	ImGui::Begin(u8"カメラ座標");

	ImGui::DragFloat3("camPos", &position.x, 1.0f, 0.0f, 10000.0f);
	ImGui::DragFloat3("camTarget", &target.x, 1.0f, 0.0f, 10000.0f);

	ImGui::End();
	//target.x += 0.1f;
	const auto eyePos = DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f);
	const auto targetPos = DirectX::XMVectorSet(target.x, target.y, target.z, 1.0f);
	const auto up = DirectX::XMVectorSet(upVec.x, upVec.y, upVec.z, 0.0f);

	const float fov = DirectX::XMConvertToRadians(fovAngle);
	const auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	viewMatrix = DirectX::XMMatrixLookAtLH(eyePos, targetPos, up);
	
	projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearFar.cameraToNear, nearFar.cameraToFar);
}

//DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 150.0f, 200.0f, 0.0f);
//DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0, 0.0f);
//DirectX::XMVECTOR upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//constexpr float fov = DirectX::XMConvertToRadians(60);
//const auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比
//
//const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
//const DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 2000.0f);

void Camera3D::SetPosition(const Vector3& _position)
{
	position = _position;
}

void Camera3D::SetTarget(const Vector3& _target)
{
	target = _target;
}

void Camera3D::SetFov(float _fav)
{
	fovAngle = _fav;
}

void Camera3D::SetNearFar(float _near, float _far)
{
	//小さい方をnear遠い方をfarにする
	nearFar.cameraToNear = max(_near, _far); 
	nearFar.cameraToFar = max(_near, _far);
}

void Camera3D::SetUpVec(const Vector3& _upVec)
{
	upVec = _upVec;
}

