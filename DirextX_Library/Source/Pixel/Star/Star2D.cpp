#include "Star2D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"
#include <math.h>

Star2D::Star2D()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	std::vector<Vertex> vertices = CreateStarVertices(100,100,100,DirectX::XMFLOAT4(1.0f, 1.0f,0.0f,0.0f));

	Vertex centerVertex;
	centerVertex.position = DirectX::XMFLOAT3(100, 100, 0.0f);
	centerVertex.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	vertices.push_back(centerVertex); // これで全11頂点

	auto vertexSize = sizeof(Vertex) * vertices.size();
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = std::make_unique<VertexBuffer>(vertexSize, vertexStride, vertices.data());
	if (!vertexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"頂点バッファの生成に失敗\n");
		assert(false);
	}

	// 中心点(10)を使って、すべての三角形を時計回りに結ぶ
	uint32_t indices[] = {
		10, 0, 1,
		10, 1, 2,
		10, 2, 3,
		10, 3, 4,
		10, 4, 5,
		10, 5, 6,
		10, 6, 7,
		10, 7, 8,
		10, 8, 9,
		10, 9, 0  // 最後は0に戻って閉じる
	};

	// インデックスバッファの生成
	auto size = sizeof(uint32_t) * std::size(indices);
	indexBuffer = std::make_unique<IndexBuffer>(size, indices);
	if (!indexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"インデックスバッファの生成に失敗\n");
	}

	indexSize = std::size(indices);

	const auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f); // 視点の位置
	const auto targetPos = DirectX::XMVectorZero(); // 視点を向ける座標
	const auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向を表すベクトル
	constexpr float fov = DirectX::XMConvertToRadians(37.5f); // 視野角
	auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
	}

	SetTriangleMatrix();

	rootSignature = std::make_unique<RootSignature>();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}

	pipelineState = std::make_unique<PipelineState>();
	pipelineState->SetInputLayout(Vertex::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());
	pipelineState->SetVS(L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS");
	pipelineState->SetPS(L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
	pipelineState->Create();

	if (!pipelineState->IsSuccess())
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
	}
}

Star2D::~Star2D()
{

}

void Star2D::Update()
{
	transform.position.x += 1.0f;
	transform.position.y += 1.0f;
	SetTriangleMatrix();
}

void Star2D::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	commandList->SetPipelineState(pipelineState->Get()); // パイプラインステートをセット
	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 三角形を描画する設定にする
	commandList->IASetVertexBuffers(0, 1, &vbView); // 頂点バッファをスロット0番を使って1個だけ設定する

	commandList->IASetIndexBuffer(&indexBuffer->View());

	commandList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0); //星のインデックスの数分描画する

}

void Star2D::SetTriangleMatrix()
{
	//ウィンドウサイズを取得
	float windowWidth = static_cast<float>(Engine::GetInstance()->GetWindowSize().width);
	float windowHeight = static_cast<float>(Engine::GetInstance()->GetWindowSize().height);

	//2D用正射影行列
	//Left=0, Right=width, Bottom=height, Top=0 に指定することで画面左上原点とする
	DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, windowWidth, windowHeight, 0.0f, 0.0f, 1.0f);

	//4.2D用ビュー行列（単位行列）
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();

	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	// ワールド行列の計算
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
		* DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		* DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = view;
		ptr->Proj = proj;
	}
}

std::vector<Vertex> Star2D::CreateStarVertices(float _centerX, float _centerY, float _radius, DirectX::XMFLOAT4 _color)
{
	const int SIZE = 10;
	std::vector<Vertex> vertices(SIZE);
	const float GoldenRatio = 0.382f;
	float r = _radius * GoldenRatio; // 黄金比に基づく内半径
	const float PI = 3.14159265358979323846f;

	for (int i = 0; i < SIZE; i++) {
		// 頂点ごとに36度（π / 5）ずつずらす。真上から始めるために - π / 2 する
		const float angle = i * (PI / 5.0f) - (PI / 2.0f);
		const float radius = (i % 2 == 0) ? _radius : r; // 偶数は外側、奇数は内側

		vertices[i].position = DirectX::XMFLOAT3(_centerX + radius * cosf(angle), _centerY + radius * sinf(angle), 0.0f);
		vertices[i].color = _color;
	}
	return vertices;
}
