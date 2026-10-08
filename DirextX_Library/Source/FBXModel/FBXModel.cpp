#include "FBXModel.h"
#include "../ShaderStruct/ShaderStruct.h"
#include "../VertexBuffer/VertexBuffer.h"
#include "../Engine/Engine.h"
#include "../ConstBuffer/ConstBuffer.h"
#include "../PipelineState/PipelineState.h"
#include "../RootSignature/RootSignature.h"
#include "../IndexBuffer/IndexBuffer.h"
#include "../DescriptorHeap/DescriptorHeap.h"
#include "../Texture/TextureLoader.h"

#include <filesystem>
namespace fs = std::filesystem;
std::wstring ReplaceExtension(const std::wstring& origin, const char* ext)
{
	fs::path p = origin.c_str();
	return p.replace_extension(ext).c_str();
}

FBXModel::FBXModel()
{

}

FBXModel::~FBXModel()
{
}

bool FBXModel::Load(const std::string& _modelFilePath)
{
	std::wstring widePath = GetWideString(_modelFilePath);
	const wchar_t* c = widePath.c_str();
	ImportSettings importSetting = // これ自体は自作の読み込み設定構造体
	{
		c,
		meshes,
		false,
		true // アリシアのモデルは、テクスチャのUVのVだけ反転してるっぽい？ので読み込み時にUV座標を逆転させる
	};

	AssimpLoader loader;
	if (!loader.Load(importSetting))
	{
		return false;
	}

	constBuffer.resize(FRAME_BUFFER_COUNT);

	Init();
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	return true;
}

bool FBXModel::Load(const wchar_t* _modelFilePath)
{
	ImportSettings importSetting = // これ自体は自作の読み込み設定構造体
	{
		_modelFilePath,
		meshes,
		false,
		true // アリシアのモデルは、テクスチャのUVのVだけ反転してるっぽい？ので読み込み時にUV座標を逆転させる
	};

	AssimpLoader loader;
	if (!loader.Load(importSetting))
	{
		return false;
	}

	constBuffer.resize(FRAME_BUFFER_COUNT);

	
	Init();
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	return true;
}

void FBXModel::Update()
{
	VertexMapping();
}

void FBXModel::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex();
	auto commandList = Engine::GetInstance()->CommandList();

	// メッシュの数だけインデックス分の描画を行う処理を回す
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = vertexBuffer[i]->GetView(); // そのメッシュに対応する頂点バッファ
		auto ibView = indexBuffer[i]->View(); // そのメッシュに対応する頂点バッファ
		auto materialHeap = DescriptorHeap::GetInstance()->GetHeap().Get();

		commandList->SetGraphicsRootSignature(rootSignature->Get());
		commandList->SetPipelineState(pipelineState->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, constBuffer[currentIndex]->GetAddress());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);

		commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i]->handleGPU); // そのメッシュに対応するディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0); // インデックスの数分描画する
	}
}

void FBXModel::VertexMapping()
{
	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	// ワールド行列の計算
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
		* DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		* DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	auto eyePos = DirectX::XMVectorSet(0.0f, 120.0, 75.0, 0.0f);
	auto targetPos = DirectX::XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constexpr float fov = DirectX::XMConvertToRadians(60);
	auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
	const DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = view;
		ptr->Proj = proj;
		ptr->uvRect = Vector4();
	}
}

void FBXModel::Init()
{
	vertexBuffer.reserve(meshes.size());

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto size = sizeof(Vertex) * meshes[i].Vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(size, stride, vertices);
		if (!pVB->IsSuccess())
		{
			OutputDebugStringW(L"頂点バッファの生成に失敗\n");
		}

		vertexBuffer.emplace_back(pVB);
	}

	useIndex = false;
	polygonSize = (UINT)vertices.size();

	// メッシュの数だけインデックスバッファを用意する
	indexBuffer.reserve(meshes.size());
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto size = sizeof(uint32_t) * meshes[i].Indices.size();
		auto indices = meshes[i].Indices.data();
		auto pIB = new IndexBuffer(size, indices);
		if (!pIB->IsSuccess())
		{
			printf("インデックスバッファの生成に失敗\n");
		}

		indexBuffer.emplace_back(pIB);
		useIndex = true;
	}

	auto eyePos = DirectX::XMVectorSet(0.0f, 120.0, 75.0, 0.0f);
	auto targetPos = DirectX::XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constexpr float fov = DirectX::XMConvertToRadians(60);
	auto aspect = static_cast<float>(Engine::GetInstance()->GetWindowSize().width) / static_cast<float>(Engine::GetInstance()->GetWindowSize().height); // アスペクト比

	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		constBuffer[i] = std::make_shared<ConstBuffer>(sizeof(MatrixTransform));
		if (!constBuffer[i]->IsValid())
		{
			OutputDebugStringW(L"変換行列用定数バッファの生成に失敗\n");
		}

		// 変換行列の登録
		auto ptr = constBuffer[i]->GetPtr<MatrixTransform>();
		ptr->World = DirectX::XMMatrixIdentity();
		ptr->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
	}

	materialHandles.clear();

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto texPath = ReplaceExtension(meshes[i].DiffuseMap, "tga");
		TextureLoader::GetInstance()->Load(texPath);
		auto mainTex = TextureLoader::GetInstance()->Get(texPath);
		auto handle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), texPath);
		materialHandles.push_back(handle);
	}

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->SetTextureRootSampler();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}


	//uvRect = { 0.0f,0.0f,1.0f,1.0f };
}

void FBXModel::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
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