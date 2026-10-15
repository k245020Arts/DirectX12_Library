#include "TriangularPrism.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

TriangularPrism::TriangularPrism()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);
	vertices.resize(24);

	length = Vector3(100.0f,100.0f,100.0f);

	SetVertices(length);

	//前面
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 上（頂点0と1を同じ位置に）
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 上
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下
	vertices[3].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左下

	
	vertices[4].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 上（裏から見て右上）
	vertices[5].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 上（裏から見て左上）
	vertices[6].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左下
	vertices[7].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下

	
	vertices[8].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 奥
	vertices[9].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 奥
	vertices[10].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 手前
	vertices[11].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 手前

	
	vertices[12].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前
	vertices[13].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前
	vertices[14].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 右奥
	vertices[15].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥

	
	vertices[16].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 奥上（頂点）
	vertices[17].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 手前上（頂点）
	vertices[18].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下
	vertices[19].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下

	
	vertices[20].color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f); // 手前上（頂点）
	vertices[21].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 奥上（頂点）
	vertices[22].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下
	vertices[23].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下


	//UV座標の設定
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
		12, 13, 14,  12, 14, 15, // 下面（底面）
		16, 17, 18,  16, 18, 19, // 左傾斜面
		20, 21, 22,  20, 22, 23  // 右傾斜面
	};

	Init(vertices, indices);
	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

TriangularPrism::~TriangularPrism()
{

}

void TriangularPrism::Update()
{
	//VertexMapping();
}

void TriangularPrism::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	if (fillMode) {
		commandList->SetPipelineState(pipelineState->GetPipelineState()); // パイプラインステートをセット
	}
	else {
		commandList->SetPipelineState(wireFramePipelineState->GetPipelineState()); // パイプラインステートをセット
	}

	commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());

	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootDescriptorTable(1, descriptorHandle->handleGPU);

	if (fillMode) {
		commandList->IASetPrimitiveTopology(drawType);//適切な形に表示をするようにする
	}
	else {
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);//ワイヤーフレームの時は線表示をする
	}

	commandList->IASetVertexBuffers(0, 1, &vbView); //頂点バッファをスロット0番を使って1個だけ設定する

	if (useIndex) {
		commandList->IASetIndexBuffer(&indexBuffer->View());

		if (fillMode) {
			commandList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0); //形に応じたインデックスの数分描画する
		}
		else {

			const UINT faceIndexCounts[] = { 4, 4, 4, 5 };

			UINT startIndex = 0;

			for (UINT count : faceIndexCounts)
			{
				commandList->DrawIndexedInstanced(count, 1, startIndex, 0, 0);

				startIndex += count;
			}
		}
		
	}
	else {
		commandList->DrawInstanced(polygonSize, 1, 0, 0); // 形に応じた頂点の数を描画する
	}
}

void TriangularPrism::SetLength(const Vector3& _length)
{
	length.x = fabs(_length.x);
	length.y = fabs(_length.y);
	length.z = fabs(_length.z);

	SetVertices(length);

	vertexBuffer->BufferMapping(vertices.data());
}

void TriangularPrism::SetFill(bool _fill)
{
	if (!_fill) {
		std::vector<uint32_t> wireFrameIndices = {
			// 前面
			0,  2,  3,  0,
			// 背面
			4,  6,  7,  4,
			// 上面
			8, 10, 11,  8,
			// 底面
			12, 13, 14, 15, 12
		};
		indexSize = (UINT)wireFrameIndices.size();
		auto size = sizeof(uint32_t) * wireFrameIndices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(wireFrameIndices.data());
	}
	else {
		// インデックス設定
		std::vector<uint32_t> indices = {
			 0,  1,  2,  0,  2,  3, // 前面
			 4,  5,  6,  4,  6,  7, // 背面
			 8,  9, 10,  8, 10, 11, // 上面
			12, 13, 14,  12, 14, 15, // 下面（底面）
			16, 17, 18,  16, 18, 19, // 左傾斜面
			20, 21, 22,  20, 22, 23  // 右傾斜面
		};
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}

	Polygon3D::SetFill(_fill);
}


void TriangularPrism::SetVertices(const Vector3& _length)
{
	float x = length.x / 2.0f;
	float y = length.y / 2.0f;
	float z = length.z / 2.0f;

	// 前面
	vertices[0].position = DirectX::XMFLOAT3(0.0f, y, -z);
	vertices[1].position = DirectX::XMFLOAT3(0.0f, y, -z);
	vertices[2].position = DirectX::XMFLOAT3(x, -y, -z);
	vertices[3].position = DirectX::XMFLOAT3(-x, -y, -z);

	// 背面
	vertices[4].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[5].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[6].position = DirectX::XMFLOAT3(-x, -y, z);
	vertices[7].position = DirectX::XMFLOAT3(x, -y, z);

	// 上面
	vertices[8].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[9].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[10].position = DirectX::XMFLOAT3(0.0f, y, -z);
	vertices[11].position = DirectX::XMFLOAT3(0.0f, y, -z);

	// 下面
	vertices[12].position = DirectX::XMFLOAT3(-x, -y, -z);
	vertices[13].position = DirectX::XMFLOAT3(x, -y, -z);
	vertices[14].position = DirectX::XMFLOAT3(x, -y, z);
	vertices[15].position = DirectX::XMFLOAT3(-x, -y, z);

	// 左傾斜面
	vertices[16].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[17].position = DirectX::XMFLOAT3(0.0f, y, -z);
	vertices[18].position = DirectX::XMFLOAT3(-x, -y, -z);
	vertices[19].position = DirectX::XMFLOAT3(-x, -y, z);

	// 右傾斜面
	vertices[20].position = DirectX::XMFLOAT3(0.0f, y, -z);
	vertices[21].position = DirectX::XMFLOAT3(0.0f, y, z);
	vertices[22].position = DirectX::XMFLOAT3(x, -y, z);
	vertices[23].position = DirectX::XMFLOAT3(x, -y, -z);
}