#include "Line2D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Line2D::Line2D()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(2);
	vertices[0].position = DirectX::XMFLOAT3(-250.0f, 0.0f, 0.0f);
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].position = DirectX::XMFLOAT3(250.0f, 0.0f, 0.0f);
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	Init(vertices, {});
	drawType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
}

Line2D::~Line2D()
{
}

void Line2D::SetLength(float _length)
{
	length = _length;

	const float LENGTH = length / 2.0f;

	vertices[0].position = DirectX::XMFLOAT3(LENGTH, 0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(-LENGTH, 0.0f, 0.0f);

	vertexBuffer->BufferMapping(vertices.data());
}


void Line2D::Update()
{
	//transform.position.x += 1.0f;
	//transform.position.y += 1.0f;
	transform.rotation.z += 0.01f;
	Set2DMatrix();
}

void Line2D::Draw()
{
	Polygon2D::Draw();
}
