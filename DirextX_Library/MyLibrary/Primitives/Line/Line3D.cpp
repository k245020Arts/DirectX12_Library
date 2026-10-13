#include "Line3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Line3D::Line3D()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(2);
	vertices[0].position = DirectX::XMFLOAT3(-250.0f, 0.0f, -50.0f);
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].position = DirectX::XMFLOAT3(250.0f, 0.0f, 50.0f);
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	Init(vertices, {});
	drawType = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Line3D::~Line3D()
{

}

void Line3D::Update()
{
	//VertexMapping();
}

void Line3D::Draw()
{
	Polygon3D::Draw();
}