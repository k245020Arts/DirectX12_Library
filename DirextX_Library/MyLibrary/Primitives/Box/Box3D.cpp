#include "Box3D.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Box3D::Box3D()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	vertices.resize(24);

	length = Vector3(100.0f, 100.0f, 100.0f);

	CreateVertexPosition();
	
	vertices[0].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左上
	vertices[1].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右上
	vertices[2].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下
	vertices[3].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左下

	vertices[4].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右上 (裏から見て)
	vertices[5].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 左上
	vertices[6].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左下
	vertices[7].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右下

	
	vertices[8].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥
	vertices[9].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 右奥
	vertices[10].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前
	vertices[11].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前

	
	vertices[12].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前
	vertices[13].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前
	vertices[14].color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f); // 右奥
	vertices[15].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥

	
	vertices[16].color = DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); // 奥上
	vertices[17].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 手前上
	vertices[18].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下
	vertices[19].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下

	
	vertices[20].color = DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f); // 手前上
	vertices[21].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 奥上
	vertices[22].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 奥下
	vertices[23].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 手前下
	
	vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 右上 (裏から見て左上)
	vertices[1].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 左上 (裏から見て右上)
	vertices[2].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 左下 (裏から見て右下)
	vertices[3].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 右下 (裏から見て左下)

	vertices[4].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 右上 (裏から見て左上)
	vertices[5].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 左上 (裏から見て右上)
	vertices[6].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 左下 (裏から見て右下)
	vertices[7].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 右下 (裏から見て左下)

	
	vertices[8].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 左奥
	vertices[9].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 右奥
	vertices[10].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 右手前
	vertices[11].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 左手前

	
	vertices[12].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 左手前
	vertices[13].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 右手前
	vertices[14].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 右奥
	vertices[15].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 左奥

	
	vertices[16].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 奥上
	vertices[17].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 手前上
	vertices[18].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 手前下
	vertices[19].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 奥下

	
	vertices[20].uv = DirectX::XMFLOAT2(0.0f, 0.0f); // 手前上
	vertices[21].uv = DirectX::XMFLOAT2(1.0f, 0.0f); // 奥上
	vertices[22].uv = DirectX::XMFLOAT2(1.0f, 1.0f); // 奥下
	vertices[23].uv = DirectX::XMFLOAT2(0.0f, 1.0f); // 手前下


	// 各面に対して、提示された「0, 1, 2, 0, 2, 3」と同じ時計回りのパターン
	std::vector<uint32_t> indices = {
		 0,  1,  2,  0,  2,  3, //前面
		 4,  5,  6,  4,  6,  7, //背面
		 8,  9, 10,  8, 10, 11, //上面
		12, 13, 14,  12, 14, 15, //下面
		16, 17, 18,  16, 18, 19, //左側面
		20, 21, 22,  20, 22, 23  //右側面
	};

	Init(vertices, indices);

	drawType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
}

Box3D::~Box3D()
{

}

void Box3D::SetLength(const Vector3& _length)
{
	length.x = fabs(_length.x);
	length.y = fabs(_length.y);
	length.z = fabs(_length.z);

	CreateVertexPosition();

	vertexBuffer->BufferMapping(vertices.data());
}

void Box3D::Update()
{
	//VertexMapping();
}

void Box3D::Draw()
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

		commandList->DrawIndexedInstanced(indexSize, 1, 0, 0, 0); //形に応じたインデックスの数分描画する
	}
	else {
		commandList->DrawInstanced(polygonSize, 1, 0, 0); // 形に応じた頂点の数を描画する
	}
}

void Box3D::CreateVertexPosition()
{
	float xLength = length.x / 2.0f;
	float yLength = length.y / 2.0f;
	float zLength = length.z / 2.0f;
	//前面
	vertices[0].position = DirectX::XMFLOAT3(-xLength, yLength, -zLength);
	vertices[1].position = DirectX::XMFLOAT3(xLength, yLength, -zLength);
	vertices[2].position = DirectX::XMFLOAT3(xLength, -yLength, -zLength);
	vertices[3].position = DirectX::XMFLOAT3(-xLength, -yLength, -zLength);

	// 背面
	vertices[4].position = DirectX::XMFLOAT3(xLength, yLength, zLength);
	vertices[5].position = DirectX::XMFLOAT3(-xLength, yLength, zLength);
	vertices[6].position = DirectX::XMFLOAT3(-xLength, -yLength, zLength);
	vertices[7].position = DirectX::XMFLOAT3(xLength, -yLength, zLength);
	
	//上面
	vertices[8].position = DirectX::XMFLOAT3(-xLength, yLength, zLength);
	vertices[9].position = DirectX::XMFLOAT3(xLength, yLength, zLength);
	vertices[10].position = DirectX::XMFLOAT3(xLength, yLength, -zLength);
	vertices[11].position = DirectX::XMFLOAT3(-xLength, yLength, -zLength);

	// 下面 
	vertices[12].position = DirectX::XMFLOAT3(-xLength, -yLength, -zLength);
	vertices[13].position = DirectX::XMFLOAT3(xLength, -yLength, -zLength);
	vertices[14].position = DirectX::XMFLOAT3(xLength, -yLength, zLength);
	vertices[15].position = DirectX::XMFLOAT3(-xLength, -yLength, zLength);

	//左側面
	vertices[16].position = DirectX::XMFLOAT3(-xLength, yLength, zLength);
	vertices[17].position = DirectX::XMFLOAT3(-xLength, yLength, -zLength);
	vertices[18].position = DirectX::XMFLOAT3(-xLength, -yLength, -zLength);
	vertices[19].position = DirectX::XMFLOAT3(-xLength, -yLength, zLength);

	//右側面
	vertices[20].position = DirectX::XMFLOAT3(xLength, yLength, -zLength);
	vertices[21].position = DirectX::XMFLOAT3(xLength, yLength, zLength);
	vertices[22].position = DirectX::XMFLOAT3(xLength, -yLength, zLength);
	vertices[23].position = DirectX::XMFLOAT3(xLength, -yLength, -zLength);
}
