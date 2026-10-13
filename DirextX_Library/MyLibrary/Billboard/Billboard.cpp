#include "Billboard.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../VertexBuffer/VertexBuffer.h"
#include "../Engine/Engine.h"
#include "../ConstBuffer/ConstBuffer.h"
#include "../PipelineState/PipelineState.h"
#include "../RootSignature/RootSignature.h"
#include "../IndexBuffer/IndexBuffer.h"
#include "../Camera3D/Camera3D.h"

Billboard::Billboard()
{
	yBillBoard = true;
	uvRect = {};
}

Billboard::~Billboard()
{
}

bool Billboard::Load(const std::string& _path)
{
	bool result = TextureLoader::GetInstance()->Load(_path);

	TextureData* texture = TextureLoader::GetInstance()->Get(_path);

	if (texture == nullptr) {
		std::string error = _path + "こちらのパスがありません";
		assert(false && error.c_str());
		return false;
	}

	constBuffer.resize(FRAME_BUFFER_COUNT);
	textureData = *texture;

	descriptorHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), _path);

	if (descriptorHandle == nullptr)
	{
		assert(false && "Descriptorの登録に失敗しました");
		return false;
	}

	vertices.resize(4);
	//verticesの中心点を0,0,0にするために半分のサイズを取得
	const float width = result ? static_cast<float>(texture->width) / 2.0f : 100;
	const float height = result ? static_cast<float>(texture->height) / 2.0f : 100;

	if (result) {
		textureSize.width = static_cast<LONG>(texture->width);
		textureSize.height = static_cast<LONG>(texture->height);
	}
	else {
		textureSize.width = 200;
		textureSize.height = 200;
	}

	vertices[0].position = { -width,  height, 0.0f };
	vertices[1].position = { width,  height, 0.0f };
	vertices[2].position = { width, -height, 0.0f };
	vertices[3].position = { -width, -height, 0.0f };

	vertices[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[2].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertices[0].uv = { 1.0f, 0.0f };
	vertices[1].uv = { 0.0f, 0.0f };
	vertices[2].uv = { 0.0f, 1.0f };
	vertices[3].uv = { 1.0f, 1.0f };

	//初期状態は画像のすべて表示
	uvRect = { 0.0f,0.0f,1.0f,1.0f };

	//fillMode = true;
	auto vertexSize = sizeof(Vertex) * std::size(vertices);
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = std::make_unique<VertexBuffer>(vertexSize, vertexStride, vertices.data());
	if (!vertexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"頂点バッファの生成に失敗\n");
	}

	//useIndex = false;
	//polygonSize = (UINT)vertices.size();

	std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 }; // これに書かれている順序で描画する

	// インデックスバッファの生成
	if (!indices.empty())
	{
		auto size = sizeof(uint32_t) * indices.size();

		indexBuffer = std::make_unique<IndexBuffer>(size, indices.data());

		//indexSize = (UINT)_indices.size();

		if (!indexBuffer->IsSuccess())
		{
			OutputDebugStringW(L"インデックスバッファの生成に失敗\n");
		}
	}

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
	}

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->SetTextureRootSampler();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/2DTextureShader/2DTextureVertexShader.hlsl", "TexVsMain", L"Shader/2DTextureShader/2DTexturePixelShader.hlsl", "TexPsMain");
	return true;
}

void Billboard::SetBillBoardType(bool _yBillboard)
{
	yBillBoard = _yBillboard;
}

void Billboard::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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

DirectX::XMMATRIX Billboard::YBillBoardMatrix(const Camera3D* _camera)
{
	const Vector3 cameraUpVec = _camera->GetUpVec();
	const DirectX::XMVECTOR upVec = DirectX::XMVectorSet(cameraUpVec.x, cameraUpVec.y, cameraUpVec.z, 0.0f);
	const Vector3 cameraPosVec3 = _camera->GetPosition();
	const DirectX::XMVECTOR cameraPosition = DirectX::XMVectorSet(cameraPosVec3.x, cameraPosVec3.y, cameraPosVec3.z, 0.0f);

	const  DirectX::XMVECTOR dist = DirectX::XMVectorSet(cameraPosVec3.x - transform.position.x, 0.0f, cameraPosVec3.z - transform.position.z, 0.0f);

	const DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(dist);

	// 右方向を計算
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(upVec, forward);

	right = DirectX::XMVector3Normalize(right);

	// ベクトルをXMFLOAT3に変換
	Vector3 rightVec;
	Vector3 forwardVec;

	DirectX::XMStoreFloat3(&rightVec, right);
	DirectX::XMStoreFloat3(&forwardVec, forward);

	const DirectX::XMMATRIX rotation = DirectX::XMMatrixSet(
		rightVec.x, rightVec.y, rightVec.z, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return rotation;
}

DirectX::XMMATRIX Billboard::AllBillBoardMatrix(const Camera3D* _camera)
{
	const Vector3 cameraUpVecVector3 = _camera->GetUpVec();
	const DirectX::XMVECTOR cameraUpVec = DirectX::XMVectorSet(cameraUpVecVector3.x, cameraUpVecVector3.y, cameraUpVecVector3.z, 0.0f);
	const Vector3 cameraPosVec3 = _camera->GetPosition();
	const DirectX::XMVECTOR cameraPosition = DirectX::XMVectorSet(cameraPosVec3.x, cameraPosVec3.y, cameraPosVec3.z, 0.0f);

	const  DirectX::XMVECTOR dist = DirectX::XMVectorSet(cameraPosVec3.x - transform.position.x, cameraPosVec3.y - transform.position.y, cameraPosVec3.z - transform.position.z, 0.0f);

	const DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(dist);

	// 右方向を計算
	DirectX::XMVECTOR right = DirectX::XMVector3Cross(cameraUpVec, forward);

	right = DirectX::XMVector3Normalize(right);

	// カメラの方向に直交する上方向を求める
	const DirectX::XMVECTOR up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, right));

	// ベクトルをXMFLOAT3に変換
	Vector3 rightVec;
	Vector3 forwardVec;
	Vector3 upVec;

	DirectX::XMStoreFloat3(&rightVec, right);
	DirectX::XMStoreFloat3(&forwardVec, forward);
	DirectX::XMStoreFloat3(&upVec, up);

	const DirectX::XMMATRIX rotation = DirectX::XMMatrixSet(
		rightVec.x, rightVec.y, rightVec.z, 0.0f,
		upVec.x, upVec.y, upVec.z, 0.0f,
		forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return rotation;
}


void Billboard::VertexMapping(const Camera3D* _camera)
{
	const DirectX::XMMATRIX rotation = yBillBoard ? YBillBoardMatrix(_camera) : AllBillBoardMatrix(_camera);

	Vector3 scale = transform.scale;
	Vector3 position = transform.position;

	// ワールド行列の計算
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
		* rotation
		* DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = _camera->GetViewMatrix();
		ptr->Proj = _camera->GetProjectionMatrix();
		ptr->uvRect = uvRect;
	}
}


void Billboard::SetRect(float _startX, float _startY, float _width, float _height)
{
	SetRect(Vector4(_startX, _startY, _width, _height));
}

void Billboard::SetRect(const Vector4& _rect)
{
	uvRect.x = _rect.x / textureSize.width;
	uvRect.y = _rect.y / textureSize.height;
	uvRect.z = _rect.z / textureSize.width;
	uvRect.w = _rect.w / textureSize.height;

	const Vector2 size = { _rect.z / 2.0f ,_rect.w / 2.0f };
	//if (!defalutSize) {
	//	return;
	//}
	vertices[0].position = { -size.x,  size.y, 0.0f };
	vertices[1].position = { size.x,  size.y, 0.0f };
	vertices[2].position = { size.x, -size.y, 0.0f };
	vertices[3].position = { -size.x, -size.y, 0.0f };

	vertexBuffer->BufferMapping(vertices.data());
}

void Billboard::SetRectUV(const Vector4& _rect)
{
	//uvの値を座標変換
	Vector4 rect = { _rect.x * textureSize.width,_rect.y * textureSize.height ,_rect.z * textureSize.width ,_rect.w * textureSize.height };
	SetRect(rect);
}


void Billboard::Update()
{
}

void Billboard::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	commandList->SetPipelineState(pipelineState->GetPipelineState()); // パイプラインステートをセット
	commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());

	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootDescriptorTable(1, descriptorHandle->handleGPU);

	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//適切な形に表示をするようにする

	commandList->IASetVertexBuffers(0, 1, &vbView); //頂点バッファをスロット0番を使って1個だけ設定する

	commandList->IASetIndexBuffer(&indexBuffer->View());

	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0); //形に応じたインデックスの数分描画する
}
