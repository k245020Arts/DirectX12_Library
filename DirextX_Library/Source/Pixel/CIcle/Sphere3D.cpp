#include "Sphere3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Sphere3D::Sphere3D()
{
	std::wstring white = L"white";
	bool result = TextureLoader::GetInstance()->ColorTexture(white, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
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

	constBuffer.resize(FRAME_BUFFER_COUNT);

	// 球体の設定パラメータ 
	const float radius = 50.0f;       //半径（キューブのサイズに合わせた大きさ）
	const uint32_t sliceCount = 32;   //経度方向の分割数
	const uint32_t stackCount = 16;   //緯度方向の分割数

	//頂点数の計算とリサイズ
	uint32_t vertexCount = (stackCount + 1) * (sliceCount + 1);
	vertices.resize(vertexCount);

	uint32_t vIndex = 0;

	//頂点データの生成
	for (uint32_t i = 0; i <= stackCount; ++i) {
		//緯度角
		float phi = DirectX::XM_PI * static_cast<float>(i) / static_cast<float>(stackCount);
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (uint32_t j = 0; j <= sliceCount; ++j) {
			//経度角
			float theta = DirectX::XM_2PI * static_cast<float>(j) / static_cast<float>(sliceCount);
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			//位置（Position）の計算
			float x = radius * sinPhi * sinTheta;
			float y = radius * cosPhi;
			float z = radius * sinPhi * cosTheta;
			vertices[vIndex].position = DirectX::XMFLOAT3(x, y, z);

			// 位置座標 (-50.0 ~ +50.0) を 0.0 ~ 1.0 の範囲に正規化して、綺麗なグラデーションを作ります
			float r = (x / radius) * 0.5f + 0.5f;
			float g = (y / radius) * 0.5f + 0.5f;
			float b = (z / radius) * 0.5f + 0.5f;
			vertices[vIndex].color = DirectX::XMFLOAT4(r, g, b, 1.0f);

			// UV座標の計算
			float u = static_cast<float>(j) / static_cast<float>(sliceCount);
			float v = static_cast<float>(i) / static_cast<float>(stackCount);
			vertices[vIndex].uv = DirectX::XMFLOAT2(u, v);

			vIndex++;
		}
	}

	//インデックスデータの生成 
	std::vector<uint32_t> indices;
	//各グリッドを時計回りの三角形2つ（インデックス6個）に分割
	for (uint32_t i = 0; i < stackCount; ++i) {
		for (uint32_t j = 0; j < sliceCount; ++j) {
			uint32_t row1 = i * (sliceCount + 1);
			uint32_t row2 = (i + 1) * (sliceCount + 1);

			// 四角形の4頂点のインデックス
			uint32_t topLeft = row1 + j;
			uint32_t topRight = row1 + j + 1;
			uint32_t bottomLeft = row2 + j;
			uint32_t bottomRight = row2 + j + 1;

			// 三角形1（左上 -> 右上 -> 右下）提示された「0, 1, 2」のパターン順
			indices.push_back(topLeft);
			indices.push_back(topRight);
			indices.push_back(bottomRight);

			// 三角形2（左上 -> 右下 -> 左下）提示された「0, 2, 3」のパターン順
			indices.push_back(topLeft);
			indices.push_back(bottomRight);
			indices.push_back(bottomLeft);
		}
	}

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Sphere3D::~Sphere3D()
{

}

void Sphere3D::Update()
{
	VertexMapping();
}

void Sphere3D::Draw()
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

	if (fillMode) {
		commandList->IASetPrimitiveTopology(drawType);//適切な形に表示をするようにする
	}
	else {
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);//ワイヤーフレームの時は線表示をする
	}

	commandList->IASetVertexBuffers(0, 1, &vbView); //頂点バッファをスロット0番を使って1個だけ設定する

	if (useIndex) {
		commandList->IASetIndexBuffer(&indexBuffer->View());

		commandList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0); //形に応じたインデックスの数分描画する
	}
	else {
		commandList->DrawInstanced(polygonSize, 1, 0, 0); // 形に応じた頂点の数を描画する
	}
}

void Sphere3D::Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
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
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
	}

	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 120.0, -120.0f, 0.0f);
	DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	DirectX::XMVECTOR upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constexpr float fov = DirectX::XMConvertToRadians(60);
	const auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}

		// 変換行列の登録
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = DirectX::XMMatrixIdentity();
		ptr->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 2000.0f);
	}

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->SetTextureRootSampler();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}

	uvRect = { 0.0f,0.0f,1.0f,1.0f };
}

void Sphere3D::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
{
	pipelineState = std::make_unique<PipelineState>(topology);
	pipelineState->SetInputLayout(Vertex::InputLayout);
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

void Sphere3D::SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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


void Sphere3D::VertexMapping()
{
	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	// ワールド行列の計算
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
		* DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		* DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 150.0f, 200.0f, 0.0f);
	DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0, 0.0f);
	DirectX::XMVECTOR upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constexpr float fov = DirectX::XMConvertToRadians(60);
	const auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
	const DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 2000.0f);

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = view;
		ptr->Proj = proj;
		ptr->uvRect = Vector4();
	}
}
