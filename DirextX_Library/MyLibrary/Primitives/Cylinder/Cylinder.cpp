#include "Cylinder.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Cylinder::Cylinder()
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

	const int SLICES = 24; // 円周の分割数 : 増やすほど滑らかな円柱
	const float RADIUS = 50.0f;
	const float HEIGHT = 100.0f;
	const float HALF_HEIGHT = HEIGHT * 0.5f;

	// 1つの四角形（面）あたり4頂点。
	// 上面下面側面の合計
	int totalFaces = SLICES + SLICES + SLICES;
	vertices.resize(totalFaces * 4);
	std::vector<uint32_t> indices;
	indices.reserve(totalFaces * 6);

	int vIndex = 0; 

	//上面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 上面は緑
		}
			
		vIndex += 4;
	}

	// 下面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), -HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), -HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 下面は赤
		}
		vIndex += 4;
	}

	//側面 
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		float c1 = cosf(theta1), s1 = sinf(theta1);
		float c2 = cosf(theta2), s2 = sinf(theta2);

		vertices[vIndex].position = DirectX::XMFLOAT3(RADIUS * c1, HALF_HEIGHT, RADIUS * s1); // 左上
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(RADIUS * c2, HALF_HEIGHT, RADIUS * s2); // 右上
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * c2, -HALF_HEIGHT, RADIUS * s2); // 右下
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * c1, -HALF_HEIGHT, RADIUS * s1); // 左下

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 側面は青
		}
		vIndex += 4;
	}

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		vertices[baseV].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		vertices[baseV + 1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
		vertices[baseV + 2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
		vertices[baseV + 3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);

		indices.push_back(baseV + 0);
		indices.push_back(baseV + 1);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 0);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 3);
	}

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Cylinder::~Cylinder()
{

}

void Cylinder::Update()
{
	VertexMapping();
}

void Cylinder::Draw()
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

void Cylinder::Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
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

void Cylinder::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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

void Cylinder::SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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


void Cylinder::VertexMapping()
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
