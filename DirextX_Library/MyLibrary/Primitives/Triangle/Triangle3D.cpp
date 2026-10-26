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

	length = Vector3(100.0f, 100.0f, 100.0f);

	CreateVertexPositionTriangle(length);

	vertices[0].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[1].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[2].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f); // 右手前下
	vertices[3].color = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // 左手前下

	
	vertices[4].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[5].color = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[6].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 左奥下
	vertices[7].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f); // 右奥下

	
	vertices[8].color = Vector4(1.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[9].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[10].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // てっぺん
	vertices[11].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん

	
	vertices[12].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 左手前下
	vertices[13].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 右手前下
	vertices[14].color = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // 右奥下
	vertices[15].color = Vector4(1.0f, 0.0f, 1.0f, 1.0f); // 左奥下

	
	vertices[16].color = Vector4(1.0f, 0.0f, 1.0f, 1.0f); // てっぺん
	vertices[17].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[18].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f); // 左手前下
	vertices[19].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 左奥下

	
	vertices[20].color = Vector4(0.0f, 1.0f, 1.0f, 1.0f); // てっぺん
	vertices[21].color = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // てっぺん
	vertices[22].color = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 右奥下
	vertices[23].color = Vector4(0.0f, 0.0f, 1.0f, 1.0f); // 右手前下


	// UV座標の設定
	for (int i = 0; i < 24; i += 4) {
		vertices[i].uv = Vector2(0.0f, 0.0f);
		vertices[i + 1].uv = Vector2(1.0f, 0.0f);
		vertices[i + 2].uv = Vector2(1.0f, 1.0f);
		vertices[i + 3].uv = Vector2(0.0f, 1.0f);
	}

	// 前面
	auto frontNormal = CalculateNormal(
		vertices[0].position,
		vertices[2].position,
		vertices[3].position);

	for (int i = 0; i < 4; ++i)
	{
		vertices[i].normal = frontNormal;
	}

	// 背面
	auto backNormal = CalculateNormal(
		vertices[4].position,
		vertices[6].position,
		vertices[7].position);

	for (int i = 4; i < 8; ++i)
	{
		vertices[i].normal = backNormal;
	}

	// 下面
	auto bottomNormal = CalculateNormal(
		vertices[12].position,
		vertices[13].position,
		vertices[14].position);

	for (int i = 12; i < 16; ++i)
	{
		vertices[i].normal = bottomNormal;
	}

	// 左側面
	auto leftNormal = CalculateNormal(
		vertices[16].position,
		vertices[18].position,
		vertices[19].position);

	for (int i = 16; i < 20; ++i)
	{
		vertices[i].normal = leftNormal;
	}

	// 右側面
	auto rightNormal = CalculateNormal(
		vertices[20].position,
		vertices[22].position,
		vertices[23].position);

	for (int i = 20; i < 24; ++i)
	{
		vertices[i].normal = rightNormal;
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
	SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
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
	commandList->SetGraphicsRootConstantBufferView(2, lightConstBuffer[currentIndex]->GetAddress()); // 定数バッファをセット

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

void Triangle3D::SetLength(const Vector3& _length)
{
	length.x = fabs(_length.x);
	length.y = fabs(_length.y);
	length.z = fabs(_length.z);

	CreateVertexPositionTriangle(length);

	vertexBuffer->BufferMapping(vertices.data());
}


void Triangle3D::SetFill(bool _fill)
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
			12, 13, 14,  12, 14, 15, // 下面
			16, 17, 18,  16, 18, 19, // 左側面
			20, 21, 22,  20, 22, 23  // 右側面
		};
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}

	Polygon3D::SetFill(_fill);
}


void Triangle3D::CreateVertexPositionTriangle(const Vector3& _length)
{
	const float x = _length.x / 2.0f;
	const float y = _length.y / 2.0f;
	const float z = _length.z / 2.0f;
	//前面
	vertices[0].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[1].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[2].position = DirectX::XMFLOAT3(x, -y, -z);
	vertices[3].position = DirectX::XMFLOAT3(-x, -y, -z);

	//背面
	vertices[4].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[5].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[6].position = DirectX::XMFLOAT3(-x, -y, z);
	vertices[7].position = DirectX::XMFLOAT3(x, -y, z);

	// 上面
	vertices[8].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[9].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[10].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[11].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);

	// 下面
	vertices[12].position = DirectX::XMFLOAT3(-x, -y, -z);
	vertices[13].position = DirectX::XMFLOAT3(x, -y, -z);
	vertices[14].position = DirectX::XMFLOAT3(x, -y, z);
	vertices[15].position = DirectX::XMFLOAT3(-x, -y, z);

	// 左側面
	vertices[16].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[17].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[18].position = DirectX::XMFLOAT3(-x, -y, -z);
	vertices[19].position = DirectX::XMFLOAT3(-x, -y, z);

	//右側面
	vertices[20].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[21].position = DirectX::XMFLOAT3(0.0f, y, 0.0f);
	vertices[22].position = DirectX::XMFLOAT3(x, -y, z);
	vertices[23].position = DirectX::XMFLOAT3(x, -y, -z);
}

Vector3 Triangle3D::CalculateNormal(const Vector3& p0,const Vector3& p1,const Vector3& p2)
{
	using namespace DirectX;

	Vector3 a = p1 - p0;
	Vector3 b = p2 - p0;

	Vector3 normal = VectorHelper::VCross(a,b);

	return normal;
}