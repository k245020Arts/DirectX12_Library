#include "Triangle3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Triangle3D::Triangle3D()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(24);

	//前面
	vertices[0].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[1].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[2].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右手前下
	vertices[3].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[3].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左手前下

	//背面
	vertices[4].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[4].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[5].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[5].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[6].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[6].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左奥下
	vertices[7].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[7].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右奥下

	// 上面
	vertices[8].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[8].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[9].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[9].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[10].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[10].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[11].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[11].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん

	// 下面
	vertices[12].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[12].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前下
	vertices[13].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[13].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前下
	vertices[14].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[14].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 右奥下
	vertices[15].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[15].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥下

	// 左側面
	vertices[16].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[16].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[17].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[17].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[18].position = DirectX::XMFLOAT3(-50.0f, -50.0f, -50.0f); vertices[18].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 左手前下
	vertices[19].position = DirectX::XMFLOAT3(-50.0f, -50.0f, 50.0f); vertices[19].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 左奥下

	//右側面
	vertices[20].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[20].color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f); // てっぺん
	vertices[21].position = DirectX::XMFLOAT3(0.0f, 50.0f, 0.0f); vertices[21].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[22].position = DirectX::XMFLOAT3(50.0f, -50.0f, 50.0f); vertices[22].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右奥下
	vertices[23].position = DirectX::XMFLOAT3(50.0f, -50.0f, -50.0f); vertices[23].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右手前下


	// UV座標の設定
	for (int i = 0; i < 24; i += 4) {
		vertices[i].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		vertices[i + 1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
		vertices[i + 2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
		vertices[i + 3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);
	}


	// インデックス設定
	std::vector<uint32_t> indices = {
		 0,  1,  2,  0,  2,  3, // 前面
		 4,  5,  6,  4,  6,  7, // 背面
		 8,  9, 10,  8, 10, 11, // 上面
		12, 13, 14,  12, 14, 15, // 下面
		16, 17, 18,  16, 18, 19, // 左側面
		20, 21, 22,  20, 22, 23  // 右側面
	};

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Triangle3D::~Triangle3D()
{

}

void Triangle3D::Update()
{
	//VertexMapping();
}

void Triangle3D::Draw()
{
	Polygon3D::Draw();
}
