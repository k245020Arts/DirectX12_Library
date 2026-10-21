#include "Cylinder.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Cylinder::Cylinder()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	baseColor = Vector3(1.0f, 1.0f, 1.0f);

	const int SLICES = 24; // 円周の分割数 : 増やすほど滑らかな円柱
	const float RADIUS = 50.0f;
	const float HEIGHT = 100.0f;
	const float HALF_HEIGHT = HEIGHT * 0.5f;

	// 1つの四角形（面）あたり4頂点。
	// 上面下面側面の合計
	int totalFaces = SLICES + SLICES + SLICES;
	vertices.resize(totalFaces * 4);
	std::vector<uint32_t> indices;
	indices.reserve(totalFaces * 6);

	int vIndex = 0; 

	//上面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 上面は緑
		}
			
		vIndex += 4;
	}

	// 下面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), -HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), -HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 下面は赤
		}
		vIndex += 4;
	}

	//側面 
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		float c1 = cosf(theta1), s1 = sinf(theta1);
		float c2 = cosf(theta2), s2 = sinf(theta2);

		vertices[vIndex].position = DirectX::XMFLOAT3(RADIUS * c1, HALF_HEIGHT, RADIUS * s1); // 左上
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(RADIUS * c2, HALF_HEIGHT, RADIUS * s2); // 右上
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * c2, -HALF_HEIGHT, RADIUS * s2); // 右下
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * c1, -HALF_HEIGHT, RADIUS * s1); // 左下

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 側面は青
		}
		vIndex += 4;
	}

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		vertices[baseV].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		vertices[baseV + 1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
		vertices[baseV + 2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
		vertices[baseV + 3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);

		indices.push_back(baseV + 0);
		indices.push_back(baseV + 1);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 0);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 3);
	}

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Cylinder::~Cylinder()
{

}

void Cylinder::SetCylinderParam(const CylinderParam& _param)
{
	std::vector<uint32_t> indices;
	param = _param;
	CreateVertexCylider(param.slices, param.radius, param.height, indices);

	if (indexSize != (UINT)indices.size()) {
		indexSize = (UINT)indices.size();

		auto size = sizeof(uint32_t) * indices.size();

		indexBuffer->Resize(size);
	}

	indexBuffer->BufferMapping(indices.data());

	if (indexSize != (UINT)vertices.size()) {

		polygonSize = (UINT)vertices.size();
		auto verSize = sizeof(Vertex) * vertices.size();
		vertexBuffer->Resize(verSize);

	}
	vertexBuffer->BufferMapping(vertices.data());

}

void Cylinder::SetFill(bool _fill)
{
	if (!_fill) {
		std::vector<uint32_t> wireFrameIndices;
		// インデックス設定
		CreateIndexWireFrame(param.slices, wireFrameIndices);
		indexSize = (UINT)wireFrameIndices.size();
		auto size = sizeof(uint32_t) * wireFrameIndices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(wireFrameIndices.data());
	}
	else {
		std::vector<uint32_t> indices;
		// インデックス設定
		CreateIndexFill(param.slices, indices);
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}

	Polygon3D::SetFill(_fill);
}

void Cylinder::Update()
{
	//VertexMapping();
}

void Cylinder::Draw()
{
	Polygon3D::Draw();
}

void Cylinder::CreateVertexCylider(int _slices, float _radius, float _height, std::vector<uint32_t>& indices)
{
	const int SLICES = _slices; // 円周の分割数 : 増やすほど滑らかな円柱
	const float RADIUS = _radius;
	const float HEIGHT = _height;
	const float HALF_HEIGHT = HEIGHT * 0.5f;

	// 1つの四角形（面）あたり4頂点。
	// 上面下面側面の合計
	int totalFaces = SLICES + SLICES + SLICES;
	vertices.resize(totalFaces * 4);

	indices.reserve(totalFaces * 6);

	int vIndex = 0;

	//上面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = Vector4(baseColor.x,baseColor.y,baseColor.z,vertices[vIndex + j].color.w);
		}

		// 上面の法線（Y軸プラス方向）
		const Vector3 topNormal(0.0f, 1.0f, 0.0f);

		for (int j = 0; j < 4; ++j)
		{
			vertices[vIndex + j].normal = topNormal;
			vertices[vIndex + j].color = Vector4(baseColor.x, baseColor.y, baseColor.z, vertices[vIndex + j].color.w);
		}


		vIndex += 4;
	}

	// 下面の蓋
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		//4つのポリゴンを生成する(左上と右上の頂点を同じにする)ルールを崩さずにするために同じ座標で点を2つ記入
		vertices[vIndex].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(0.0f, -HALF_HEIGHT, 0.0f); // 中心
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), -HALF_HEIGHT, RADIUS * sinf(theta1)); // 外周1
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), -HALF_HEIGHT, RADIUS * sinf(theta2)); // 外周2

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = Vector4(baseColor.x, baseColor.y, baseColor.z, vertices[vIndex + j].color.w);
		}

		// 下面の法線（Y軸マイナス方向）
		const DirectX::XMFLOAT3 bottomNormal(0.0f, -1.0f, 0.0f);

		for (int j = 0; j < 4; ++j)
		{
			vertices[vIndex + j].normal = bottomNormal;
			vertices[vIndex + j].color = Vector4(baseColor.x, baseColor.y, baseColor.z, vertices[vIndex + j].color.w);
		}

		vIndex += 4;
	}

	//側面 
	for (int i = 0; i < SLICES; ++i) {
		float theta1 = (2.0f * DirectX::XM_PI * i) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (i + 1)) / SLICES;

		float c1 = cosf(theta1), s1 = sinf(theta1);
		float c2 = cosf(theta2), s2 = sinf(theta2);

		vertices[vIndex].position = DirectX::XMFLOAT3(RADIUS * c1, HALF_HEIGHT, RADIUS * s1); // 左上
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(RADIUS * c2, HALF_HEIGHT, RADIUS * s2); // 右上
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * c2, -HALF_HEIGHT, RADIUS * s2); // 右下
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * c1, -HALF_HEIGHT, RADIUS * s1); // 左下

		DirectX::XMFLOAT3 normal1(c1, 0.0f, s1);
		DirectX::XMFLOAT3 normal2(c2, 0.0f, s2);

		vertices[vIndex].normal = normal1;
		vertices[vIndex + 1].normal = normal2;
		vertices[vIndex + 2].normal = normal2;
		vertices[vIndex + 3].normal = normal1;

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = Vector4(baseColor.x, baseColor.y, baseColor.z, vertices[vIndex + j].color.w);
		}
		vIndex += 4;
	}

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		vertices[baseV].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		vertices[baseV + 1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
		vertices[baseV + 2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
		vertices[baseV + 3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);
	}

	
	

	if (fillMode) {
		CreateIndexFill(SLICES, indices);
	}
	else {
		CreateIndexWireFrame(SLICES, indices);
	}
}

void Cylinder::CreateIndexFill(int _slices, std::vector<uint32_t>& indices)
{
	const int TOTAL = _slices + _slices + _slices;
	// UV座標の設定
	for (int f = 0; f < TOTAL; ++f) {
		int baseV = f * 4;

		indices.push_back(baseV + 0);
		indices.push_back(baseV + 1);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 0);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 3);
	}
}

void Cylinder::CreateIndexWireFrame(int _slices, std::vector<uint32_t>& indices)
{
	indices.clear();

	const int SLICES = _slices;

	const int TOP_OFFSET = 0;
	const int BOTTOM_OFFSET = SLICES * 4;
	const int SIDE_OFFSET = SLICES * 8;

	// 上面の円周
	for (int i = 0; i < SLICES; ++i)
	{
		const uint32_t current = TOP_OFFSET + i * 4 + 3;

		const uint32_t next = TOP_OFFSET + i * 4 + 2;

		indices.push_back(current);
		indices.push_back(next);
	}

	// 下面の円周
	for (int i = 0; i < SLICES; ++i)
	{
		const uint32_t current = BOTTOM_OFFSET + i * 4 + 2;

		const uint32_t next = BOTTOM_OFFSET + i * 4 + 3;

		indices.push_back(current);
		indices.push_back(next);
	}

	
	// 側面の縦線
	for (uint32_t i = 0; i < SLICES; ++i)
	{
		const uint32_t base = SIDE_OFFSET + i * 4;

		indices.push_back(base);
        indices.push_back(base + 3);
	}
}