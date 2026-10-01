#include "Pixel.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"

Pixel::Pixel()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	Vertex vertices[1] = {};
	vertices[0].position = DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f); // 上
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	auto vertexSize = sizeof(Vertex) * std::size(vertices);
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = std::make_unique<VertexBuffer>(vertexSize, vertexStride, vertices);
	if (!vertexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"頂点バッファの生成に失敗\n");
		assert(false);
	}

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

	pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
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

Pixel::~Pixel()
{
}


void Pixel::Update()
{
	//transform.position.x += 1.0f;
	//transform.position.y += 1.0f;
	SetTriangleMatrix();
}

void Pixel::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	commandList->SetPipelineState(pipelineState->Get()); // パイプラインステートをセット
	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット

	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST); // ポリゴンを描画する設定にする
	commandList->IASetVertexBuffers(0, 1, &vbView); // 頂点バッファをスロット0番を使って1個だけ設定する

	commandList->DrawInstanced(1, 1, 0, 0); // 1個の頂点を描画する

}

void Pixel::SetTriangleMatrix()
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
