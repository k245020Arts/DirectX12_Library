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
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Cylinder::~Cylinder()
{

}

void Cylinder::Update()
{
	//VertexMapping();
}

void Cylinder::Draw()
{
	Polygon3D::Draw();
}
