#include "Box3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Box3D::Box3D()
{
	std::wstring white = L"white";
	bool result = TextureLoader::GetInstance()->ColorTexture(white,Vector4(1.0f,1.0f,1.0f,1.0f));
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

	vertices.resize(24);

	//前面
	vertices[0].position = DirectX::XMFLOAT3(-50.0f, 50.0f, -50.0f); vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左上
	vertices[1].position = DirectX::XMFLOAT3(50.0f, 50.0f, -50.0f); vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右上
	vertices[2].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下
	vertices[3].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[3].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左下

	// 背面
	vertices[4].position = DirectX::XMFLOAT3(50.0f, 50.0f, 50.0f); vertices[4].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右上 (裏から見て)
	vertices[5].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 50.0f); vertices[5].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左上
	vertices[6].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[6].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左下
	vertices[7].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[7].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下

	//上面
	vertices[8].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 50.0f); vertices[8].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥
	vertices[9].position = DirectX::XMFLOAT3(50.0f, 50.0f, 50.0f); vertices[9].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右奥
	vertices[10].position = DirectX::XMFLOAT3(50.0f, 50.0f, -50.0f); vertices[10].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前
	vertices[11].position = DirectX::XMFLOAT3(-50.0f, 50.0f, -50.0f); vertices[11].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前

	// 下面 
	vertices[12].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[12].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前
	vertices[13].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[13].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前
	vertices[14].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[14].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 右奥
	vertices[15].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[15].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥

	//左側面
	vertices[16].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 50.0f); vertices[16].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 奥上
	vertices[17].position = DirectX::XMFLOAT3(-50.0f, 50.0f, -50.0f); vertices[17].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 手前上
	vertices[18].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[18].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下
	vertices[19].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[19].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下

	//右側面
	vertices[20].position = DirectX::XMFLOAT3(50.0f, 50.0f, -50.0f); vertices[20].color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f); // 手前上
	vertices[21].position = DirectX::XMFLOAT3(50.0f, 50.0f, 50.0f); vertices[21].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 奥上
	vertices[22].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[22].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下
	vertices[23].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[23].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下
	
	vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 右上 (裏から見て左上)
	vertices[1].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 左上 (裏から見て右上)
	vertices[2].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 左下 (裏から見て右下)
	vertices[3].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 右下 (裏から見て左下)

	vertices[4].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 右上 (裏から見て左上)
	vertices[5].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 左上 (裏から見て右上)
	vertices[6].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 左下 (裏から見て右下)
	vertices[7].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 右下 (裏から見て左下)

	
	vertices[8].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 左奥
	vertices[9].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 右奥
	vertices[10].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 右手前
	vertices[11].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 左手前

	
	vertices[12].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 左手前
	vertices[13].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 右手前
	vertices[14].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 右奥
	vertices[15].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 左奥

	
	vertices[16].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 奥上
	vertices[17].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 手前上
	vertices[18].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 手前下
	vertices[19].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 奥下

	
	vertices[20].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 手前上
	vertices[21].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 奥上
	vertices[22].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 奥下
	vertices[23].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 手前下


	// 各面に対して、提示された「0, 1, 2, 0, 2, 3」と同じ時計回りのパターン
	std::vector<uint32_t> indices = {
		 0,  1,  2,  0,  2,  3, //前面
		 4,  5,  6,  4,  6,  7, //背面
		 8,  9, 10,  8, 10, 11, //上面
		12, 13, 14,  12, 14, 15, //下面
		16, 17, 18,  16, 18, 19, //左側面
		20, 21, 22,  20, 22, 23  //右側面
	};

	Init(vertices, indices);

	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Box3D::~Box3D()
{

}

void Box3D::Update()
{
	VertexMapping();
}

void Box3D::Draw()
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

void Box3D::Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
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

void Box3D::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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

void Box3D::SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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


void Box3D::VertexMapping()
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
