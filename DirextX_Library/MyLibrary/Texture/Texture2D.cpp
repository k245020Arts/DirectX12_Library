#include "Texture2D.h"
#include "TextureLoader.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../VertexBuffer/VertexBuffer.h"
#include "../Engine/Engine.h"
#include "../ConstBuffer/ConstBuffer.h"
#include "../PipelineState/PipelineState.h"
#include "../RootSignature/RootSignature.h"
#include "../IndexBuffer/IndexBuffer.h"

Texture2D::Texture2D()
{
	constBuffer.resize(FRAME_BUFFER_COUNT);
	defalutSize = true;
}

Texture2D::~Texture2D()
{
	delete descriptorHandle;
	descriptorHandle = nullptr;
}

bool Texture2D::Load(std::string _path)
{
	bool result = TextureLoader::GetInstance()->Load(_path);

	TextureData* texture = TextureLoader::GetInstance()->Get(_path);
	
	if (texture == nullptr) {
		std::string error = _path + "こちらのパスがありません";
		assert(false &&  error.c_str());
		return false;
	}

	textureData = *texture;
	
	descriptorHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), _path);

    if (descriptorHandle == nullptr)
    {
        assert(false && "Descriptorの登録に失敗しました");
        return false;
    }

	vertices.resize(4);
	//verticesの中心点を0,0,0にするために半分のサイズを取得
	const float width = result ? static_cast<float>(texture->width) / 2.0f : 100;
	const float height = result ?  static_cast<float>(texture->height) / 2.0f : 100;

	if (result) {
		textureSize.width = static_cast<LONG>(texture->width);
		textureSize.height = static_cast<LONG>(texture->height);
	}
	else {
		textureSize.width = 200;
		textureSize.height = 200;
	}

	vertices[0].position = { -width,  height, 0.0f };
	vertices[1].position = { width,  height, 0.0f };
	vertices[2].position = { width, -height, 0.0f };
	vertices[3].position = { -width, -height, 0.0f };

	vertices[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[2].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertices[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };

	vertices[0].uv = { 0.0f, 1.0f };
	vertices[1].uv = { 1.0f, 1.0f };
	vertices[2].uv = { 1.0f, 0.0f };
	vertices[3].uv = { 0.0f, 0.0f };

	//初期状態は画像のすべて表示
	uvRect = { 0.0f,0.0f,1.0f,1.0f };

	//fillMode = true;
	auto vertexSize = sizeof(Vertex) * std::size(vertices);
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = std::make_unique<VertexBuffer>(vertexSize, vertexStride, vertices.data());
	if (!vertexBuffer->IsSuccess())
	{
		OutputDebugStringW(L"頂点バッファの生成に失敗\n");
	}

	//useIndex = false;
	//polygonSize = (UINT)vertices.size();

	std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 }; // これに書かれている順序で描画する

	// インデックスバッファの生成
	if (!indices.empty())
	{
		auto size = sizeof(uint32_t) * indices.size();

		indexBuffer = std::make_unique<IndexBuffer>(size, indices.data());

		//indexSize = (UINT)_indices.size();

		if (!indexBuffer->IsSuccess())
		{
			OutputDebugStringW(L"インデックスバッファの生成に失敗\n");
		}
	}

	const auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f); // 視点の位置
	const auto targetPos = DirectX::XMVectorZero(); // 視点を向ける座標
	const auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向を表すベクトル
	constexpr float fov = DirectX::XMConvertToRadians(37.5f); // 視野角
	auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}
	}

	Set2DMatrix();

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->SetTextureRootSampler();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}

	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/2DTextureShader/2DTextureVertexShader.hlsl", "TexVsMain", L"Shader/2DTextureShader/2DTexturePixelShader.hlsl", "TexPsMain");
	return true;
}

void Texture2D::SetSize(int _width, int _height)
{
	SetSize(static_cast<float>(_width), static_cast<float>(_height));
}

void Texture2D::SetSize()
{
	SetSize(static_cast<float>(textureSize.width), static_cast<float>(textureSize.height));
	defalutSize = false;
}

void Texture2D::SetColor(const Vector4& _color)
{
	vertices[0].color = _color;
	vertices[1].color = _color;
	vertices[2].color = _color;
	vertices[3].color = _color;

	vertexBuffer->BufferMapping(vertices.data());
}

void Texture2D::Update()
{
	//transform.rotation.z += 0.01f;
	Set2DMatrix();
}

void Texture2D::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する
	auto commandList = Engine::GetInstance()->CommandList(); // コマンドリスト
	auto vbView = vertexBuffer->GetView(); // 頂点バッファビュー

	commandList->SetGraphicsRootSignature(rootSignature->Get()); // ルートシグネチャをセット
	commandList->SetPipelineState(pipelineState->GetPipelineState()); // パイプラインステートをセット
	commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());
	
	commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootDescriptorTable(1, descriptorHandle->handleGPU);

	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//適切な形に表示をするようにする

	commandList->IASetVertexBuffers(0, 1, &vbView); //頂点バッファをスロット0番を使って1個だけ設定する

	commandList->IASetIndexBuffer(&indexBuffer->View());

	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0); //形に応じたインデックスの数分描画する

}

void Texture2D::Set2DMatrix()
{
	//ウィンドウサイズを取得
	float windowWidth = static_cast<float>(Engine::GetInstance()->GetWindowSize().width);
	float windowHeight = static_cast<float>(Engine::GetInstance()->GetWindowSize().height);

	//2D用正射影行列
	//Left=0, Right=width, Bottom=height, Top=0 に指定することで画面左上原点とする
	DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, windowWidth, windowHeight, 0.0f, 0.0f, 1.0f);

	//4.2D用ビュー行列（単位行列）
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();

	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	// ワールド行列の計算
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
		* DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		* DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = view;
		ptr->Proj = proj;
		ptr->uvRect = uvRect;
	}
}

void Texture2D::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
{
	pipelineState = std::make_unique<PipelineState>(topology);
	pipelineState->SetInputLayout(Vertex::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());
	pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
	pipelineState->SetVS(_VSfilePath, _VSentryPoint);
	pipelineState->SetPS(_PSfilePath, _PSentryPoint);
	pipelineState->Create();

	if (!pipelineState->IsSuccess())
	{
		OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
	}
}

void Texture2D::SetBlendMode(BlendState _blendState)
{
	pipelineState->SetBlendMode(_blendState);
}

void Texture2D::SetAlpha(float _alphaValue)
{
	const float alpha = _alphaValue >= 1.0f ? 1.0f : _alphaValue;
	for (auto& vertex : vertices) {
		vertex.color.w = _alphaValue;
	}
	vertexBuffer->BufferMapping(vertices.data());
}

void Texture2D::SetRect(float _startX, float _startY, float _width, float _height)
{
	SetRect(Vector4(_startX, _startY, _width, _height));
}

void Texture2D::SetRect(const Vector4& _rect)
{
	uvRect.x = _rect.x / textureSize.width;
	uvRect.y = _rect.y / textureSize.height;
	uvRect.z = _rect.z / textureSize.width;
	uvRect.w = _rect.w / textureSize.height;

	const Vector2 size = { _rect.z / 2.0f ,_rect.w / 2.0f };
	if (!defalutSize) {
		return;
	}
	vertices[0].position = { -size.x,  size.y, 0.0f };
	vertices[1].position = { size.x,  size.y, 0.0f };
	vertices[2].position = { size.x, -size.y, 0.0f };
	vertices[3].position = { -size.x, -size.y, 0.0f };

	vertexBuffer->BufferMapping(vertices.data());
}

void Texture2D::SetRectUV(const Vector4& _rect)
{
	//uvの値を座標変換
	Vector4 rect = { _rect.x * textureSize.width,_rect.y * textureSize.height ,_rect.z * textureSize.width ,_rect.w * textureSize.height };
	SetRect(rect);
}

void Texture2D::SetFlipX(bool flip)
{
	flipX = flip;
	UpdateUV();
}

void Texture2D::SetFlipY(bool flip)
{
	flipY = flip;
	UpdateUV();
}

void Texture2D::SetSize(float _width, float _height)
{
	defalutSize = false;
	const Vector2 size = { _width / 2.0f ,_height / 2.0f };

	vertices[0].position = { -size.x,  size.y, 0.0f };
	vertices[1].position = { size.x,  size.y, 0.0f };
	vertices[2].position = { size.x, -size.y, 0.0f };
	vertices[3].position = { -size.x, -size.y, 0.0f };

	vertexBuffer->BufferMapping(vertices.data());
}

void Texture2D::UpdateUV()
{
	const float u0 = flipX ? 1.0f : 0.0f;
	const float u1 = flipX ? 0.0f : 1.0f;

	const float v0 = flipY ? 0.0f : 1.0f;
	const float v1 = flipY ? 1.0f : 0.0f;

	//反転をする必要がある個所をひとまとめにしてuvの設定をする
	vertices[0].uv = { u0, v0 };
	vertices[1].uv = { u1, v0 };
	vertices[2].uv = { u1, v1 };
	vertices[3].uv = { u0, v1 };

	vertexBuffer->BufferMapping(vertices.data());
}
