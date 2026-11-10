#include "Polygon3D.h"
#include "../VertexBuffer/VertexBuffer.h"
#include "../Engine/Engine.h"
#include "../ConstBuffer/ConstBuffer.h"
#include "../PipelineState/PipelineState.h"
#include "../RootSignature/RootSignature.h"
#include "../IndexBuffer/IndexBuffer.h"
#include "../Texture/TextureLoader.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../Camera3D/Camera3D.h"
#include "../Shadow/ShadowCamera.h"
#include "../Shadow/ShadowMap.h"

Polygon3D::Polygon3D()
{
}

Polygon3D::~Polygon3D()
{
}

void Polygon3D::Update()
{
}

void Polygon3D::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	if (fillMode) {
		commandList->SetPipelineState(pipelineState->GetPipelineState()); // パイプラインステートをセット
	}
	else {
		commandList->SetPipelineState(wireFramePipelineState->GetPipelineState()); // パイプラインステートをセット
	}

	commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());

	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootDescriptorTable(1, descriptorHandle->handleGPU);
	commandList->SetGraphicsRootConstantBufferView(2, lightConstBuffer[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootDescriptorTable(3, shadowMapHandle->handleGPU);

	if (fillMode) {
		commandList->IASetPrimitiveTopology(drawType);//適切な形に表示をするようにする
	}
	else {
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);//ワイヤーフレームの時は線表示をする
	}

	commandList->IASetVertexBuffers(0, 1, &vbView); //頂点バッファをスロット0番を使って1個だけ設定する

	if (useIndex) {
		commandList->IASetIndexBuffer(&indexBuffer->View());
		if (groundShadow){
			commandList->DrawIndexedInstanced(indexSize, 2, 0, 0, 0); //形に応じたインデックスの数分描画する
		}
		else {
			commandList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0); //形に応じたインデックスの数分描画する
		}
		
	}
	else {
		commandList->DrawInstanced(polygonSize, 1, 0, 0); // 形に応じた頂点の数を描画する
	}
}

void Polygon3D::DrawShadow(const ShadowCamera& camera, const ShadowMap& _shadowMap)
{
	shadowMapHandle = _shadowMap.GetSRV();
	if (shadowPipelineState == nullptr || !shadowPipelineState->IsSuccess() || shadowConstBuffer.empty())
	{
		return;
	}

	const UINT currentIndex = Engine::GetInstance()->CurrentBackBufferIndex();
	if (currentIndex >= shadowConstBuffer.size())
	{
		return;
	}

	auto* commandList = Engine::GetInstance()->CommandList();

	auto* transformData = shadowConstBuffer[currentIndex]->GetPtr<MatrixTransform>();
	Matrix world = Matrix::Scale(transform.scale) * Matrix::Rotation(transform.rotation) * Matrix::Translation(transform.position);
	transformData->World = world;
	transformData->View = camera.GetViewMatrix();
	transformData->Proj = camera.GetProjectionMatrix();
	transformData->uvRect = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	Matrix lightViewProjection = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	transformData->lightViewProjection = lightViewProjection;

	auto* constBuff = constBuffer[currentIndex]->GetPtr<MatrixTransform>();
	constBuff->lightViewProjection = lightViewProjection;
	//constBuff->useShadow = useShadowMap;

	commandList->SetGraphicsRootSignature(shadowRootSignature->Get());
	commandList->SetPipelineState(shadowPipelineState->GetPipelineState());

	// ShadowDepthRootSignature: RP0=b0 Transform, RP1=b2 BoneBuffer。
	commandList->SetGraphicsRootConstantBufferView(0, shadowConstBuffer[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const auto vbView = vertexBuffer->GetView();

	commandList->IASetVertexBuffers(0, 1, &vbView);
	if (useIndex) {
		commandList->IASetIndexBuffer(&indexBuffer->View());
		commandList->DrawIndexedInstanced(static_cast<UINT>(indexSize), 1, 0, 0, 0);
	}
	else {
		commandList->DrawInstanced(static_cast<UINT>(polygonSize), 1, 0, 0);
	}
	
}

void Polygon3D::Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
	shadowConstBuffer.resize(FRAME_BUFFER_COUNT);
	lightConstBuffer.resize(FRAME_BUFFER_COUNT);
	fillMode = true;
	auto vertexSize = sizeof(Vertex) * std::size(_vertices);
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = std::make_unique<VertexBuffer>(vertexSize, vertexStride, _vertices.data());
	if (!vertexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"頂点バッファの生成に失敗\n");
	}

	useIndex = false;
	polygonSize = (UINT)vertices.size();

	// インデックスバッファの生成
	if (!_indices.empty())
	{
		auto size = sizeof(uint32_t) * _indices.size();

		indexBuffer = std::make_unique<IndexBuffer>(size, _indices.data());

		indexSize = (UINT)_indices.size();

		if (!indexBuffer->IsSuccess())
		{
			OutputDebugStringW(L"インデックスバッファの生成に失敗\n");
		}
		useIndex = true;
	}

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		shadowConstBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		lightConstBuffer[i] = std::make_shared<ConstBuffer>(sizeof(LightBufferData));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
		if (!lightConstBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"ライト用定数バッファの生成に失敗\n");
		}
		auto light = lightConstBuffer[i]->GetPtr<LightBufferData>();
		LightBufferData l = Light::GetInstance()->GetLightBufferData();
		*light = l;
	}

	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 120.0, -120.0f, 0.0f);
	DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	DirectX::XMVECTOR upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constexpr float fov = DirectX::XMConvertToRadians(60);
	const auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		// 変換行列の登録
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = DirectX::XMMatrixIdentity();
		ptr->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 2000.0f);
		ptr->shadow = DirectX::XMMatrixIdentity();

		auto shadowPtr = shadowConstBuffer[i]->GetPtr<MatrixTransform>();
		shadowPtr->World = DirectX::XMMatrixIdentity();
		shadowPtr->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		shadowPtr->Proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 2000.0f);
		shadowPtr->shadow = DirectX::XMMatrixIdentity();
	}

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->Set3DRootSignature();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}
	SetShadowPipelineState();
	uvRect = { 0.0f,0.0f,1.0f,1.0f };
}

void Polygon3D::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
{
	pipelineState = std::make_unique<PipelineState>(topology);
	pipelineState->SetInputLayout(VertexSkinning3D::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());
	pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
	pipelineState->SetVS(_VSfilePath, _VSentryPoint);
	pipelineState->SetPS(_PSfilePath, _PSentryPoint);
	pipelineState->Create();

	if (!pipelineState->IsSuccess())
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
	}
}

void Polygon3D::SetShadowPipelineState()
{
	shadowRootSignature = std::make_unique<RootSignature>();
	shadowRootSignature->SetShadowDepthRootSignature();
	if (!shadowRootSignature->IsSuccess())
	{
		OutputDebugStringW(L"Shadow用RootSignatureの生成に失敗\n");
		return;
	}

	shadowPipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	shadowPipelineState->SetInputLayout(Vertex::InputLayout);
	shadowPipelineState->SetRootSignature(shadowRootSignature->Get());
	shadowPipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
	shadowPipelineState->SetVS(L"Shader/3DBaseShader/Polygon3DDepthVertexShader.hlsl", "DepthVS_Polygon3D");
	shadowPipelineState->CreateDepthOnly();

	if (!shadowPipelineState->IsSuccess())
	{
		OutputDebugStringW(L"Shadow用PipelineStateの生成に失敗\n");
	}
}

void Polygon3D::SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
{
	wireFramePipelineState = std::make_unique<PipelineState>(topology);
	wireFramePipelineState->SetInputLayout(Vertex::InputLayout);
	wireFramePipelineState->SetRootSignature(rootSignature->Get());
	wireFramePipelineState->SetDrawLayOut(D3D12_FILL_MODE_WIREFRAME);
	wireFramePipelineState->SetVS(_VSfilePath, _VSentryPoint);
	wireFramePipelineState->SetPS(_PSfilePath, _PSentryPoint);
	wireFramePipelineState->Create();

	if (!wireFramePipelineState->IsSuccess())
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
	}
}


void Polygon3D::CreateWhiteTextureHandle()
{
	std::wstring white = L"white";
	ResultErrorHandle::ErrorCode result = TextureLoader::GetInstance()->ColorTexture(white, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	//bool result = TextureLoader::GetInstance()->Load("data/textest.png");

	TextureData* texture = TextureLoader::GetInstance()->Get(white);
	//TextureData* texture = TextureLoader::GetInstance()->Get("data/textest.png");

	if (texture == nullptr) {
		std::string error = "白色のパスがありません";
		assert(false && error.c_str());
	}

	textureData = *texture;

	descriptorHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), white);
	//descriptorHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), "data/textest.png");

	if (descriptorHandle == nullptr)
	{
		assert(false && "Descriptorの登録に失敗しました");
	}
}

void Polygon3D::VertexMapping(const Camera3D* _camera)
{
	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	// ワールド行列の計算
	Matrix world = Matrix::Scale(scale)
		* Matrix::Rotation(rotation)
		* Matrix::Translation(position);

	Vector4 plane = Vector4(0, 1, 0, 0);
	Vector3 lightVec = Light::GetInstance()->GetLightBufferData().lights[0].direction * -1.0f;
	Matrix shadow = DirectX::XMMatrixShadow(DirectX::XMLoadFloat4(&plane), DirectX::XMLoadFloat3(&lightVec));

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = _camera->GetViewMatrix();
		ptr->Proj = _camera->GetProjectionMatrix();
		ptr->uvRect = Vector4();
		ptr->shadow = shadow;

		auto light = lightConstBuffer[i]->GetPtr<LightBufferData>();
		LightBufferData l = Light::GetInstance()->GetLightBufferData();
		*light = l;
	}
}

void Polygon3D::SetBlendMode(BlendState _blendState)
{
	pipelineState->SetBlendMode(_blendState);
}

void Polygon3D::SetAlpha(float _alphaValue)
{
	const float alpha = _alphaValue >= 1.0f ? 1.0f : _alphaValue;
	for (auto& vertex : vertices) {
		vertex.color.w = _alphaValue;
	}

	vertexBuffer->BufferMapping(vertices.data());
}

void Polygon3D::SetFill(bool _fill)
{
	fillMode = _fill;
}

void Polygon3D::SetColor(const Rgb& _rgb)
{
	SetColor(Color::RGBToVector3(_rgb));
}

void Polygon3D::SetColor(const Vector3& _color)
{
	for (auto& vertex : vertices) {
		vertex.color = Vector4(_color.x, _color.y, _color.z, vertex.color.w);
	}

	baseColor = _color;

	vertexBuffer->BufferMapping(vertices.data());
}