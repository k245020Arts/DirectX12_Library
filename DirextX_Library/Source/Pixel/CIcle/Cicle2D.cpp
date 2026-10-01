#include "Cicle2D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Cicle2D::Cicle2D()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);
	cicleWireFrameConstBuffer.resize(FRAME_BUFFER_COUNT);

	// 頂点を4つにして四角形を定義する
	vertices.resize(4);

	vertices[0].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 0.0f);
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(50.0f, 50.0f, 0.0f);
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[2].position = DirectX::XMFLOAT3(50.0f, -50.0f, 0.0f);
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	vertices[2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[3].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 0.0f);
	vertices[3].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	vertices[3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);

	std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 }; //これに書かれている順序で描画する

	Init(vertices, indices);
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		cicleWireFrameConstBuffer[i] = std::make_shared<ConstBuffer>(sizeof(CicleWireFrame));
		if (!cicleWireFrameConstBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
	}

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DCiclePixelShader/2DCiclePixelShader.hlsl", "Cicle2DPixelShader");
}

Cicle2D::~Cicle2D()
{
	
	
}

void Cicle2D::SetFill(bool _fill)
{
	fill = _fill;
	FillConstShaderUpdate();
	//Polygon2D::SetFill(_fill);
}

void Cicle2D::FillConstShaderUpdate()
{
	if (!fill) {
		for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			auto ptr = cicleWireFrameConstBuffer[i]->GetPtr<CicleWireFrame>();
			ptr->wireFrame = 1.0f;
		}
	}
	else {
		for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			auto ptr = cicleWireFrameConstBuffer[i]->GetPtr<CicleWireFrame>();
			ptr->wireFrame = 0.0f;
		}
	}
}

void Cicle2D::SetRadius(float _radius)
{
	const float RADOIS = _radius;
	radius = _radius;
	vertices[0].position = DirectX::XMFLOAT3(-RADOIS, RADOIS, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(RADOIS, RADOIS, 0.0f);

	vertices[2].position = DirectX::XMFLOAT3(RADOIS, -RADOIS, 0.0f);

	vertices[3].position = DirectX::XMFLOAT3(-RADOIS, -RADOIS, 0.0f);

	vertexBuffer->BufferMapping(vertices.data());
}

void Cicle2D::Update()
{
	transform.position.x += 1.0f;
	transform.position.y += 1.0f;
	//transform.rotation.z += 0.01f;
	Set2DMatrix();
	FillConstShaderUpdate();
}

void Cicle2D::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー
	
	Polygon2D::Draw();

	commandList->SetGraphicsRootConstantBufferView(0, cicleWireFrameConstBuffer[currentIndex]->GetAddress()); // 定数バッファをセット

}