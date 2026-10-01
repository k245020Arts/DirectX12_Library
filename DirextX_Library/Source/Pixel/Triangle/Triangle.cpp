#include "Triangle.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Triangle::Triangle()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(3);
	vertices[0].position = DirectX::XMFLOAT3(0.0f, -50.0f, 0.0f); // è„
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].position = DirectX::XMFLOAT3(50.0f, 50.0f, 0.0f); // âEâ∫
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 0.0f); // ç∂â∫
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	Init(vertices, {});

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");

	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

Triangle::~Triangle()
{

}

void Triangle::Update()
{
	transform.position.x += 1.0f;
	transform.position.y += 1.0f;
	transform.rotation.z += 0.01f;
	Set2DMatrix();
}

void Triangle::Draw() 
{
	Polygon2D::Draw();
}