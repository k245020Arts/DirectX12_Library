#include "Box2D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Box2D::Box2D()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);

	// ’¸“_‚ð4‚Â‚É‚µ‚ÄŽlŠpŒ`‚ð’è‹`‚·‚é
	vertices.resize(4);
	vertices[0].position = DirectX::XMFLOAT3(-50.0f, 50.0f, 0.0f);
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].position = DirectX::XMFLOAT3(50.0f, 50.0f, 0.0f);
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(50.0f, -50.0f, 0.0f);
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	vertices[3].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 0.0f);
	vertices[3].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);

	std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 }; // ‚±‚ê‚É‘‚©‚ê‚Ä‚¢‚é‡˜‚Å•`‰æ‚·‚é

	Init(vertices, indices);
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
}

Box2D::~Box2D()
{

}

void Box2D::SetLength(const Vector2& _length)
{
	length = _length;

	const Vector2 LENGTH = { _length.x / 2.0f,_length.y / 2.0f };

	vertices[0].position = DirectX::XMFLOAT3(-LENGTH.x, LENGTH.y, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(LENGTH.x, LENGTH.y, 0.0f);

	vertices[2].position = DirectX::XMFLOAT3(LENGTH.x, -LENGTH.y, 0.0f);

	vertices[3].position = DirectX::XMFLOAT3(-LENGTH.x, -LENGTH.y, 0.0f);

	vertexBuffer->BufferMapping(vertices.data());
}

void Box2D::SetFill(bool _fill)
{
	if (!_fill) {
		std::vector<uint32_t> indices = { 0, 1, 2,3,0 }; // ‚±‚ê‚É‘‚©‚ê‚Ä‚¢‚é‡˜‚Å•`‰æ‚·‚é
		indexSize = indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}
	else {
		std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 }; // ‚±‚ê‚É‘‚©‚ê‚Ä‚¢‚é‡˜‚Å•`‰æ‚·‚é
		indexSize = indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}
	
	Polygon2D::SetFill(_fill);
}

void Box2D::Update()
{
	transform.position.x += 1.0f;
	transform.position.y += 1.0f;
	//transform.rotation.z += 0.01f;
	transform.rotation.z += 0.01f;
	Set2DMatrix();
}

void Box2D::Draw()
{
	Polygon2D::Draw();
}