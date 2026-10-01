#include "Pixel.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Pixel::Pixel()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(1);
	vertices[0].position = DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f); // è„
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	Init(vertices, {});

	drawType = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
}

Pixel::~Pixel()
{
}


void Pixel::Update()
{
	//transform.position.x += 1.0f;
	//transform.position.y += 1.0f;
	Set2DMatrix();
}

void Pixel::Draw()
{
	Polygon2D::Draw();
}