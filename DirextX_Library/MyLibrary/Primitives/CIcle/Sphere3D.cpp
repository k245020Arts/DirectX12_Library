#include "Sphere3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Sphere3D::Sphere3D()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	// 球体の設定パラメータ 
	const float radius = 50.0f;       //半径（キューブのサイズに合わせた大きさ）
	const uint32_t sliceCount = 32;   //経度方向の分割数
	const uint32_t stackCount = 16;   //緯度方向の分割数

	//頂点数の計算とリサイズ
	uint32_t vertexCount = (stackCount + 1) * (sliceCount + 1);
	vertices.resize(vertexCount);

	uint32_t vIndex = 0;

	//頂点データの生成
	for (uint32_t i = 0; i <= stackCount; ++i) {
		//緯度角
		float phi = DirectX::XM_PI * static_cast<float>(i) / static_cast<float>(stackCount);
		float sinPhi = sinf(phi);
		float cosPhi = cosf(phi);

		for (uint32_t j = 0; j <= sliceCount; ++j) {
			//経度角
			float theta = DirectX::XM_2PI * static_cast<float>(j) / static_cast<float>(sliceCount);
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			//位置（Position）の計算
			float x = radius * sinPhi * sinTheta;
			float y = radius * cosPhi;
			float z = radius * sinPhi * cosTheta;
			vertices[vIndex].position = DirectX::XMFLOAT3(x, y, z);

			// 位置座標 (-50.0 ~ +50.0) を 0.0 ~ 1.0 の範囲に正規化して、綺麗なグラデーションを作ります
			float r = (x / radius) * 0.5f + 0.5f;
			float g = (y / radius) * 0.5f + 0.5f;
			float b = (z / radius) * 0.5f + 0.5f;
			vertices[vIndex].color = DirectX::XMFLOAT4(r, g, b, 1.0f);

			// UV座標の計算
			float u = static_cast<float>(j) / static_cast<float>(sliceCount);
			float v = static_cast<float>(i) / static_cast<float>(stackCount);
			vertices[vIndex].uv = DirectX::XMFLOAT2(u, v);

			vIndex++;
		}
	}

	//インデックスデータの生成 
	std::vector<uint32_t> indices;
	//各グリッドを時計回りの三角形2つ（インデックス6個）に分割
	for (uint32_t i = 0; i < stackCount; ++i) {
		for (uint32_t j = 0; j < sliceCount; ++j) {
			uint32_t row1 = i * (sliceCount + 1);
			uint32_t row2 = (i + 1) * (sliceCount + 1);

			// 四角形の4頂点のインデックス
			uint32_t topLeft = row1 + j;
			uint32_t topRight = row1 + j + 1;
			uint32_t bottomLeft = row2 + j;
			uint32_t bottomRight = row2 + j + 1;

			// 三角形1（左上 -> 右上 -> 右下）提示された「0, 1, 2」のパターン順
			indices.push_back(topLeft);
			indices.push_back(topRight);
			indices.push_back(bottomRight);

			// 三角形2（左上 -> 右下 -> 左下）提示された「0, 2, 3」のパターン順
			indices.push_back(topLeft);
			indices.push_back(bottomRight);
			indices.push_back(bottomLeft);
		}
	}

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Sphere3D::~Sphere3D()
{

}

void Sphere3D::Update()
{
	//VertexMapping();
}

void Sphere3D::Draw()
{
	Polygon3D::Draw();
}