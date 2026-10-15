#include "Capsule.h"
#include "../../VertexBuffer/VertexBuffer.h"
#include "../../Engine/Engine.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../PipelineState/PipelineState.h"
#include "../../RootSignature/RootSignature.h"
#include "../../IndexBuffer/IndexBuffer.h"

Capsule::Capsule()
{
	CreateWhiteTextureHandle();

	constBuffer.resize(FRAME_BUFFER_COUNT);

	baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	const int SLICES = 24;      // 横方向（円周）の分割数
	const int STACKS = 8;       // 縦方向（半球のステップ）の分割数
	const float RADIUS = 30.0f;  // カプセルの太さ（半径）
	const float CYLINDER_HEIGHT = 60.0f; // 中央の円柱部分の長さ
	const float HALF_CYL_HEIGHT = CYLINDER_HEIGHT * 0.5f;

	capsuleParam = CapsuleParam(SLICES, STACKS, RADIUS, CYLINDER_HEIGHT);

	// 1つの四角形あたり4頂点。
	// 上半球下半球円柱側面
	int totalFaces = (SLICES * STACKS) + (SLICES * STACKS) + SLICES;
	vertices.resize(totalFaces * 4);

	std::vector<uint32_t> indices;
	indices.reserve(totalFaces * 6);

	int vIndex = 0; // 頂点配列の書き込みカウンター

	//上半球
	for (int y = 0; y < STACKS; ++y) {
		float phi1 = (DirectX::XM_PIDIV2 * y) / STACKS;
		float phi2 = (DirectX::XM_PIDIV2 * (y + 1)) / STACKS;

		for (int x = 0; x < SLICES; ++x) {
			float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
			float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

			DirectX::XMFLOAT3 p0(RADIUS * sinf(phi1) * cosf(theta1), RADIUS * cosf(phi1) + HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta1));
			DirectX::XMFLOAT3 p1(RADIUS * sinf(phi1) * cosf(theta2), RADIUS * cosf(phi1) + HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta2));
			DirectX::XMFLOAT3 p2(RADIUS * sinf(phi2) * cosf(theta2), RADIUS * cosf(phi2) + HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta2));
			DirectX::XMFLOAT3 p3(RADIUS * sinf(phi2) * cosf(theta1), RADIUS * cosf(phi2) + HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta1));

			vertices[vIndex].position = p0;
			vertices[vIndex + 1].position = p1;
			vertices[vIndex + 2].position = p2;
			vertices[vIndex + 3].position = p3;

			for (int j = 0; j < 4; ++j) {
				vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f); // 上半球は緑
			}

			vIndex += 4;
		}
	}

	//中央の円柱側面
	for (int x = 0; x < SLICES; ++x) {
		float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

		vertices[vIndex].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), HALF_CYL_HEIGHT, RADIUS * sinf(theta1)); // 左上
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), HALF_CYL_HEIGHT, RADIUS * sinf(theta2)); // 右上
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), -HALF_CYL_HEIGHT, RADIUS * sinf(theta2)); // 右下
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), -HALF_CYL_HEIGHT, RADIUS * sinf(theta1)); // 左下

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // 胴体は青
		}
		vIndex += 4;
	}

	// 下半球
	for (int y = 0; y < STACKS; ++y) {
		float phi1 = DirectX::XM_PIDIV2 + (DirectX::XM_PIDIV2 * y) / STACKS;
		float phi2 = DirectX::XM_PIDIV2 + (DirectX::XM_PIDIV2 * (y + 1)) / STACKS;

		for (int x = 0; x < SLICES; ++x) {
			float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
			float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

			DirectX::XMFLOAT3 p0(RADIUS * sinf(phi1) * cosf(theta1), RADIUS * cosf(phi1) - HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta1));
			DirectX::XMFLOAT3 p1(RADIUS * sinf(phi1) * cosf(theta2), RADIUS * cosf(phi1) - HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta2));
			DirectX::XMFLOAT3 p2(RADIUS * sinf(phi2) * cosf(theta2), RADIUS * cosf(phi2) - HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta2));
			DirectX::XMFLOAT3 p3(RADIUS * sinf(phi2) * cosf(theta1), RADIUS * cosf(phi2) - HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta1));

			vertices[vIndex].position = p0;
			vertices[vIndex + 1].position = p1;
			vertices[vIndex + 2].position = p2;
			vertices[vIndex + 3].position = p3;

			for (int j = 0; j < 4; ++j) {
				vertices[vIndex + j].color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // 下半球は赤
			}
			vIndex += 4;
		}
	}

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		// UV座標のパターン適用
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

Capsule::~Capsule()
{

}

void Capsule::SetCapsuleParam(const CapsuleParam& _param)
{
	std::vector<uint32_t> indices;
	capsuleParam = _param;

	CreateCausulePosition(capsuleParam.stacks, capsuleParam.slices, capsuleParam.radius, capsuleParam.cylinderHeight, indices);

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

void Capsule::SetFill(bool _fill)
{
	if (!_fill) {
		std::vector<uint32_t> wireFrameIndices;
		// インデックス設定
		CreateWireFrameIndices(capsuleParam.stacks, capsuleParam.slices, wireFrameIndices);
		indexSize = (UINT)wireFrameIndices.size();
		auto size = sizeof(uint32_t) * wireFrameIndices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(wireFrameIndices.data());
	}
	else {
		std::vector<uint32_t> indices;
		// インデックス設定
		CreateFillIndices(capsuleParam.stacks, capsuleParam.slices, indices);
		indexSize = (UINT)indices.size();
		auto size = sizeof(uint32_t) * indices.size();
		indexBuffer->Resize(size);
		indexBuffer->BufferMapping(indices.data());
	}

	Polygon3D::SetFill(_fill);
}

void Capsule::Update()
{
	//VertexMapping();
}

void Capsule::Draw()
{
	Polygon3D::Draw();
}

void Capsule::CreateCausulePosition(const int _stacks, const int _slices, const float _radius, const float _height, std::vector<uint32_t>& indices)
{
	const int SLICES = _slices;      // 横方向（円周）の分割数
	const int STACKS = _stacks;       // 縦方向（半球のステップ）の分割数
	const float RADIUS = _radius;  // カプセルの太さ（半径）
	const float CYLINDER_HEIGHT = _height; // 中央の円柱部分の長さ
	const float HALF_CYL_HEIGHT = CYLINDER_HEIGHT * 0.5f;

	// 1つの四角形あたり4頂点。
	// 上半球下半球円柱側面
	int totalFaces = (SLICES * STACKS) + (SLICES * STACKS) + SLICES;
	vertices.resize(totalFaces * 4);

	indices.reserve(totalFaces * 6);

	int vIndex = 0; // 頂点配列の書き込みカウンター

	//上半球
	for (int y = 0; y < STACKS; ++y) {
		float phi1 = (DirectX::XM_PIDIV2 * y) / STACKS;
		float phi2 = (DirectX::XM_PIDIV2 * (y + 1)) / STACKS;

		for (int x = 0; x < SLICES; ++x) {
			float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
			float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

			DirectX::XMFLOAT3 p0(RADIUS * sinf(phi1) * cosf(theta1), RADIUS * cosf(phi1) + HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta1));
			DirectX::XMFLOAT3 p1(RADIUS * sinf(phi1) * cosf(theta2), RADIUS * cosf(phi1) + HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta2));
			DirectX::XMFLOAT3 p2(RADIUS * sinf(phi2) * cosf(theta2), RADIUS * cosf(phi2) + HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta2));
			DirectX::XMFLOAT3 p3(RADIUS * sinf(phi2) * cosf(theta1), RADIUS * cosf(phi2) + HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta1));

			vertices[vIndex].position = p0;
			vertices[vIndex + 1].position = p1;
			vertices[vIndex + 2].position = p2;
			vertices[vIndex + 3].position = p3;

			for (int j = 0; j < 4; ++j) {
				vertices[vIndex + j].color = baseColor;
			}

			vIndex += 4;
		}
	}

	//中央の円柱側面
	for (int x = 0; x < SLICES; ++x) {
		float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
		float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

		vertices[vIndex].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), HALF_CYL_HEIGHT, RADIUS * sinf(theta1)); // 左上
		vertices[vIndex + 1].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), HALF_CYL_HEIGHT, RADIUS * sinf(theta2)); // 右上
		vertices[vIndex + 2].position = DirectX::XMFLOAT3(RADIUS * cosf(theta2), -HALF_CYL_HEIGHT, RADIUS * sinf(theta2)); // 右下
		vertices[vIndex + 3].position = DirectX::XMFLOAT3(RADIUS * cosf(theta1), -HALF_CYL_HEIGHT, RADIUS * sinf(theta1)); // 左下

		for (int j = 0; j < 4; ++j) {
			vertices[vIndex + j].color = baseColor;
		}
		vIndex += 4;
	}

	// 下半球
	for (int y = 0; y < STACKS; ++y) {
		float phi1 = DirectX::XM_PIDIV2 + (DirectX::XM_PIDIV2 * y) / STACKS;
		float phi2 = DirectX::XM_PIDIV2 + (DirectX::XM_PIDIV2 * (y + 1)) / STACKS;

		for (int x = 0; x < SLICES; ++x) {
			float theta1 = (2.0f * DirectX::XM_PI * x) / SLICES;
			float theta2 = (2.0f * DirectX::XM_PI * (x + 1)) / SLICES;

			DirectX::XMFLOAT3 p0(RADIUS * sinf(phi1) * cosf(theta1), RADIUS * cosf(phi1) - HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta1));
			DirectX::XMFLOAT3 p1(RADIUS * sinf(phi1) * cosf(theta2), RADIUS * cosf(phi1) - HALF_CYL_HEIGHT, RADIUS * sinf(phi1) * sinf(theta2));
			DirectX::XMFLOAT3 p2(RADIUS * sinf(phi2) * cosf(theta2), RADIUS * cosf(phi2) - HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta2));
			DirectX::XMFLOAT3 p3(RADIUS * sinf(phi2) * cosf(theta1), RADIUS * cosf(phi2) - HALF_CYL_HEIGHT, RADIUS * sinf(phi2) * sinf(theta1));

			vertices[vIndex].position = p0;
			vertices[vIndex + 1].position = p1;
			vertices[vIndex + 2].position = p2;
			vertices[vIndex + 3].position = p3;

			for (int j = 0; j < 4; ++j) {
				vertices[vIndex + j].color = baseColor;
			}
			vIndex += 4;
		}
	}

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		// UV座標のパターン適用
		vertices[baseV].uv = DirectX::XMFLOAT2(0.0f, 0.0f);
		vertices[baseV + 1].uv = DirectX::XMFLOAT2(1.0f, 0.0f);
		vertices[baseV + 2].uv = DirectX::XMFLOAT2(1.0f, 1.0f);
		vertices[baseV + 3].uv = DirectX::XMFLOAT2(0.0f, 1.0f);
	}

	if (fillMode) {
		CreateFillIndices(STACKS, SLICES, indices);
	}
	else {
		CreateWireFrameIndices(STACKS, SLICES, indices);
	}
}

void Capsule::CreateFillIndices(const int _stacks, const int _slices, std::vector<uint32_t>& indices)
{
	int totalFaces = (_slices * _stacks) + (_slices * _stacks) + _slices;

	// UV座標の設定
	for (int f = 0; f < totalFaces; ++f) {
		int baseV = f * 4;

		indices.push_back(baseV + 0);
		indices.push_back(baseV + 1);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 0);
		indices.push_back(baseV + 2);
		indices.push_back(baseV + 3);
	}
}

void Capsule::CreateWireFrameIndices(const int _stacks,const int _slices,std::vector<uint32_t>& indices)
{
	const int STACKS = _stacks;
	const int SLICES = _slices;

	// 各パーツの面数
	const int HEMISPHERE_FACES = STACKS * SLICES;
	const int CYLINDER_FACES = SLICES;

	// 頂点配列内の開始位置
	const int TOP_OFFSET = 0;
	const int CYLINDER_OFFSET = HEMISPHERE_FACES * 4;
	const int BOTTOM_OFFSET = (HEMISPHERE_FACES + CYLINDER_FACES) * 4;

	indices.clear();

	// 1面あたり4辺を描画するため、8個のインデックス
	indices.reserve((HEMISPHERE_FACES + CYLINDER_FACES + HEMISPHERE_FACES) * 8);

	auto AddQuadEdges = [&indices](uint32_t base)
	{
		// 0番頂点 - 1番頂点
		indices.push_back(base);
		indices.push_back(base + 1);

		// 1番頂点 - 2番頂点
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		// 2番頂点 - 3番頂点
		indices.push_back(base + 2);
		indices.push_back(base + 3);

		// 3番頂点 - 0番頂点
		indices.push_back(base + 3);
		indices.push_back(base);
	};

	// 上半球
	for (int i = 0; i < HEMISPHERE_FACES; ++i)
	{
		AddQuadEdges(TOP_OFFSET + i * 4);
	}

	// 円柱側面
	for (int i = 0; i < CYLINDER_FACES; ++i)
	{
		AddQuadEdges(CYLINDER_OFFSET + i * 4);
	}

	// 下半球
	for (int i = 0; i < HEMISPHERE_FACES; ++i)
	{
		AddQuadEdges(BOTTOM_OFFSET + i * 4);
	}
}