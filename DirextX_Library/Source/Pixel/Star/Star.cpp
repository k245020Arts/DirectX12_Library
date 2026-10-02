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

	vertices = CreateStarVertices(0,0,100,DirectX::XMFLOAT4(1.0f, 1.0f,0.0f,0.0f));

	Vertex centerVertex;
	centerVertex.position = DirectX::XMFLOAT3(0, 0, 0.0f);
	centerVertex.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	vertices.push_back(centerVertex);

	// 中心点(10)を使って、すべての三角形を時計回りに結ぶ
	std::vector<uint32_t> indices = {
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

	Init(vertices, indices);
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,L"Shader/2DBaseShader/Basic_VertexShader.hlsl", "BasicVS", L"Shader/2DBaseShader/Basic_PixelShader.hlsl", "BasicPS");
}

Star2D::~Star2D()
{

}

void Star2D::Update()
{
	/*transform.position.x += 1.0f;
	transform.position.y += 1.0f;
	transform.rotation.z += 0.01f;*/
	Set2DMatrix();
}

void Star2D::Draw()
{
	Polygon2D::Draw();
}

void Star2D::SetRadius(float _radius)
{
	radius = _radius;

	vertices = CreateStarVertices(0.0f, 0.0f, radius, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));

	// 中心頂点を追加
	Vertex centerVertex;
	centerVertex.position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	centerVertex.color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices.push_back(centerVertex);

	auto vertexSize = sizeof(Vertex) * vertices.size();

	vertexBuffer->BufferMapping(vertices.data());
}

void Star2D::SetFill(bool _fill)
{
	if (!_fill) {
		std::vector<uint32_t> indices = {
		0,1,2,3,4,5,6,7,8,9,0
		};
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}
	else {
		std::vector<uint32_t> indices = {
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
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->BufferMapping(indices.data());
	}

	Polygon2D::SetFill(_fill);
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
