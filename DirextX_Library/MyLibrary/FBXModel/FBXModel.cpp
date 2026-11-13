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
#include "../Camera3D/Camera3D.h"
#include "../Animator/Animator.h"
#include "../Time/DeltaTime.h"
#include <filesystem>
#include "../Shadow/ShadowCamera.h"
#include "../Shadow/ShadowMapCreater.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"

namespace fs = std::filesystem;
namespace {
	std::wstring ReplaceExtension(const std::wstring& origin, const char* ext)
	{
		fs::path p = origin.c_str();
		return p.replace_extension(ext).c_str();
	}

	// VertexMapping と同じ式のワールド行列
	Matrix MakeWorldMatrix(const Transform& t)
	{
		return Matrix::Scale(t.scale)
			* Matrix::Rotation(t.rotation)
			* Matrix::Translation(t.position);
	}

	Vector3 ToModelLocal(const Matrix& world, const Vector3& worldPos)
	{
		const DirectX::XMVECTOR p = DirectX::XMVector3TransformCoord(
			DirectX::XMLoadFloat3(&worldPos),
			DirectX::XMMatrixInverse(nullptr, world));

		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, p);
		return out;
	}

	// 無効のときに項目をグレーアウトする（ImGui 1.84 以降。古い版では何もしない）
	void BeginDisabledIf(bool _disabled)
	{
#if defined(IMGUI_VERSION_NUM) && IMGUI_VERSION_NUM >= 18400
		ImGui::BeginDisabled(_disabled);
#else
		(void)_disabled;
#endif
	}

	void EndDisabledIf()
	{
#if defined(IMGUI_VERSION_NUM) && IMGUI_VERSION_NUM >= 18400
		ImGui::EndDisabled();
#endif
	}

	// 直前の項目の横に「(?)」を出し、マウスを乗せると説明を表示する
	void HelpTooltip(const char* _text)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s", _text);
		}
	}

}

FBXModel::FBXModel()
{
	Detach();                                    // 自分が誰かに付いているなら外れる

	for (FBXModel* child : attachedChildren)     // 自分に付いているものを、取り外す
	{
		if (child)
		{
			child->attachParent = nullptr;
			child->attachNode = -1;
		}
	}

	animator = nullptr;
}

FBXModel::~FBXModel()
{

}

ResultErrorHandle FBXModel::Load(const std::string& _modelFilePath)
{
	return Load(GetWideString(_modelFilePath).c_str(), false, false);
}

ResultErrorHandle FBXModel::Load(const std::string& _modelFilePath, bool _uflip, bool _vflip)
{
	return Load(GetWideString(_modelFilePath).c_str(), _uflip, _vflip);
}

ResultErrorHandle FBXModel::Load(const wchar_t* _modelFilePath)
{
	return Load(_modelFilePath, false, false);
}

ResultErrorHandle FBXModel::Load(const wchar_t* _modelFilePath, bool _uflip, bool _vflip)
{
	tag = WideToStringUTF8(_modelFilePath);
	ResultErrorHandle handle;
	handle.SetErrorCode(ResultErrorHandle::ErrorCode::OK);
	if (_modelFilePath == nullptr)
	{
		handle.SetErrorCode(ResultErrorHandle::ErrorCode::InvalidPath);
		return handle;
	}

	ImportSettings importSetting = // これ自体は自作の読み込み設定構造体
	{
		_modelFilePath,
		_uflip,
		_vflip
	};

	AssimpLoader loader;
	handle.SetErrorCode(loader.Load(importSetting, modelData));
	if (!handle.IsSuccess())
	{
		return handle;
	}

	if (animator == nullptr) {
		animator = std::make_unique<Animator>(modelData);
	}
	else {
		animator->AddAnimation(modelData);
	}

	Init();
	drawType = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/FBXModelShader/FBXModelVertexShader.hlsl", "BasicVS_FBX", L"Shader/FBXModelShader/FBXModelPixelShader.hlsl", "BasicPS_FBX");
	//SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, L"Shader/3DBaseShader/3DBaseVertexShader.hlsl", "BasicVS_3D", L"Shader/3DBaseShader/3DBasePixelShader.hlsl", "BasicPS_3D");
	if (!loader.GetTextureLoader()) {
		pipelineState->ErrorStateChange();
	}
	return handle;
}

ResultErrorHandle FBXModel::LoadAnimation(const std::string& _modelFilePath)
{
	return LoadAnimation(GetWideString(_modelFilePath));
}

ResultErrorHandle FBXModel::LoadAnimation(const std::wstring& _modelFilePath)
{
	AssimpLoader loader;
	ResultErrorHandle handle;
	handle.SetErrorCode(loader.LoadAnimation(_modelFilePath, modelData));
	if (!handle.IsSuccess()) {
		return handle;
	}

	if (animator == nullptr) {
		animator = std::make_unique<Animator>(modelData);
	}
	else {
		animator->AddAnimation(modelData);
	}

	return handle;
}

void FBXModel::Update()
{
	animator->Update(DeltaTime::GetInstance()->GetDeltaTimeMulTimeScale());

	/*Matrix matrix = Matrix::Rotation(transform.rotation);

	transform.position += matrix.TransformDirection(Vector3(0.0f, 0.0f, 2.0f));*/
}

void FBXModel::Draw()
{
	auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex();
	auto commandList = Engine::GetInstance()->CommandList();

	// メッシュの数だけインデックス分の描画を行う処理を回す
	for (size_t i = 0; i < modelData.meshes.size(); i++)
	{
		auto vbView = vertexBuffer[i]->GetView(); // そのメッシュに対応する頂点バッファ
		auto ibView = indexBuffer[i]->View(); // そのメッシュに対応する頂点バッファ
		auto materialHeap = DescriptorHeap::GetInstance()->GetHeap().Get();

		commandList->SetDescriptorHeaps(1, DescriptorHeap::GetInstance()->GetHeap().GetAddressOf());

		commandList->SetGraphicsRootSignature(rootSignature->Get());
		commandList->SetPipelineState(pipelineState->GetPipelineState());
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferPtr.matrixTransform[currentIndex]->GetAddress()); // 定数バッファをセット
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i].diffuseMap->handleGPU); // そのメッシュに対応するディスクリプタテーブルをセット
		commandList->SetGraphicsRootConstantBufferView(2, constantBufferPtr.light[currentIndex]->GetAddress()); // 定数バッファをセット
		commandList->SetGraphicsRootConstantBufferView(3, constantBufferPtr.bone[currentIndex]->GetAddress()); // 定数バッファをセット

		if (shadowMapHandle != nullptr) {
			commandList->SetGraphicsRootDescriptorTable(4, shadowMapHandle->handleGPU);
		}

		commandList->SetGraphicsRootDescriptorTable(5, materialHandles[i].normalMap->handleGPU); // そのメッシュに対応するディスクリプタテーブルをセット
		commandList->SetGraphicsRootConstantBufferView(6, constantBufferPtr.normalMap[currentIndex]->GetAddress()); // 定数バッファをセット
		commandList->SetGraphicsRootConstantBufferView(7, constantBufferPtr.specularMap[currentIndex]->GetAddress()); // 定数バッファをセット

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);

		
		
		//地面影を描画する場合はインシタンシングを2にする。
		if (groundShadow) {
			commandList->DrawIndexedInstanced((UINT)modelData.meshes[i].Indices.size(), 2, 0, 0, 0); // インデックスの数分描画する
		}
		else {
			commandList->DrawIndexedInstanced((UINT)modelData.meshes[i].Indices.size(), 1, 0, 0, 0); // インデックスの数分描画する
		}

		
	}
}

void FBXModel::PlayAnimation(int animIndex, bool loop, float blendSeconds)
{
	animator->Play(animIndex, loop, blendSeconds);
}

void FBXModel::StopAnimation()
{
	animator->Stop();
}

void FBXModel::SetAnimationSpeed(float s)
{
	animator->SetSpeed(s);
}

bool FBXModel::GetAnimationPlaying()
{
	return animator->GetPlaying();
}


Matrix FBXModel::GetNodeWorldMatrix(const std::wstring& nodeName) const
{
	if (!animator)
		return BuildWorldMatrix();

	// モデルのローカル → ワールド（行ベクトル: ローカル × ワールド行列）
	return animator->GetNodeModelMatrix(nodeName) * BuildWorldMatrix();
}

Vector3 FBXModel::GetNodeWorldPosition(const std::wstring& nodeName) const
{
	const Matrix m = GetNodeWorldMatrix(nodeName);

	Vector3 p;
	DirectX::XMStoreFloat3(&p, m.r[3]);
	return Vector3(p.x, p.y, p.z);
}

int FBXModel::GetNodeIndex(const std::wstring& nodeName) const
{
	return animator ? animator->GetNodeIndex(nodeName) : -1;
}

const std::wstring& FBXModel::GetNodeName(int nodeIndex) const
{
	static const std::wstring empty;
	return animator ? animator->GetNodeName(nodeIndex) : empty;
}

int FBXModel::GetNodeCount() const
{
	return animator ? animator->GetNodeCount() : 0;
}

int FBXModel::GetAnimationIndex()
{
	return animator->GetAnimationIndex();
}

// ---------------- IK ----------------

int FBXModel::AddIK(const std::wstring& name, IKSolverType type, const std::vector<std::wstring>& nodeNames)
{
	return animator ? animator->AddIK(name, type, nodeNames) : -1;
}

int FBXModel::FindIK(const std::wstring& name) const
{
	return animator ? animator->FindIK(name) : -1;
}

void FBXModel::SetIKTarget(int ikId, const Vector3& worldPos, float weight)
{
	if (animator) animator->SetIKTarget(ikId, ToModelLocal(BuildWorldMatrix(), worldPos), weight);
}

void FBXModel::SetIKTargetNode(int ikId, const std::wstring& nodeName, float weight)
{
	if (animator) animator->SetIKTargetNode(ikId, nodeName, weight);
}

void FBXModel::SetIKPole(int ikId, const Vector3& worldPos)
{
	if (animator) animator->SetIKPole(ikId, ToModelLocal(BuildWorldMatrix(), worldPos));
}

void FBXModel::SetIKPoleNode(int ikId, const std::wstring& nodeName)
{
	if (animator) animator->SetIKPoleNode(ikId, nodeName);
}

void FBXModel::ClearIKPole(int ikId)
{
	if (animator) animator->ClearIKPole(ikId);
}

void FBXModel::SetIKHinge(int ikId, int linkIndex, const Vector3& boneLocalAxis, float minAngle, float maxAngle)
{
	if (animator) animator->SetIKHinge(ikId, linkIndex, boneLocalAxis, minAngle, maxAngle);
}

void FBXModel::SetIKWeight(int ikId, float weight)
{
	if (animator) animator->SetIKWeight(ikId, weight);
}

void FBXModel::SetIKEnabled(int ikId, bool enabled)
{
	if (animator) animator->SetIKEnabled(ikId, enabled);
}

float FBXModel::GetIKHingeAngle(int ikId, int linkIndex) const
{
	return animator ? animator->GetIKHingeAngle(ikId, linkIndex) : 0.0f;
}

IKChain* FBXModel::GetIK(int ikId)
{
	return animator ? animator->GetIK(ikId) : nullptr;
}

void FBXModel::Init()
{
	vertexBuffer.reserve(modelData.meshes.size());
	for (size_t i = 0; i < modelData.meshes.size(); i++)
	{
		auto size = sizeof(VertexSkinning3D) * modelData.meshes[i].Vertices.size();
		auto stride = sizeof(VertexSkinning3D);
		auto vertices = modelData.meshes[i].Vertices.data();
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
	indexBuffer.reserve(modelData.meshes.size());
	for (size_t i = 0; i < modelData.meshes.size(); i++)
	{
		auto size = sizeof(uint32_t) * modelData.meshes[i].Indices.size();
		auto indices = modelData.meshes[i].Indices.data();
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
		CreateConstantBufferData<MatrixTransform>(constantBufferPtr.matrixTransform[i], L"変換行列用定数バッファの生成に失敗\n");
		CreateConstantBufferData<LightBufferData>(constantBufferPtr.light[i], L"ライト用定数バッファの生成に失敗\n");
		CreateConstantBufferData<BoneBuffer>(constantBufferPtr.bone[i], L"ボーン用バッファの生成に失敗\n");
		CreateConstantBufferData<MatrixTransform>(constantBufferPtr.shadow[i], L"Shadow用変換定数バッファの生成に失敗\n");
		CreateConstantBufferData<SpecularParam>(constantBufferPtr.specularMap[i], L"スペキュラマップ用定数バッファの生成に失敗\n");
		CreateConstantBufferData<NormalMapParam>(constantBufferPtr.normalMap[i], L"ノーマルマップ用定数バッファの生成に失敗\n");
		
		// 変換行列の登録
		auto ptr = constantBufferPtr.matrixTransform[i]->GetPtr<MatrixTransform>();
		ptr->World = DirectX::XMMatrixIdentity();
		ptr->View = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
		ptr->shadow = DirectX::XMMatrixIdentity();

		auto shadowPtr = constantBufferPtr.shadow[i]->GetPtr<MatrixTransform>();
		shadowPtr->World = DirectX::XMMatrixIdentity();
		shadowPtr->View = DirectX::XMMatrixIdentity();
		shadowPtr->Proj = DirectX::XMMatrixIdentity();
		shadowPtr->uvRect = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

		auto light = constantBufferPtr.light[i]->GetPtr<LightBufferData>();
		LightBufferData l = Light::GetInstance()->GetLightBufferData();
		*light = l;

		auto* bone = constantBufferPtr.bone[i]->GetPtr<BoneBuffer>();
		const auto& matrices = animator->GetBoneMatrices();

		const size_t count = std::min(matrices.size(), static_cast<size_t>(BONE_MAX));

		for (size_t j = 0; j < count; ++j)
		{
			DirectX::XMStoreFloat4x4(&bone->bones[j], matrices[j]);
		}

		auto specular = constantBufferPtr.specularMap[i]->GetPtr<SpecularParam>();
		*specular = specularParam;

		auto normal = constantBufferPtr.normalMap[i]->GetPtr<NormalMapParam>();
		*normal = normalMapParam;
	}

	materialHandles.clear();

	for (size_t i = 0; i < modelData.meshes.size(); i++)
	{
		auto diffusePath = modelData.meshes[i].DiffuseMap;
		if (!modelData.meshes[i].DiffuseMap.empty()) {
			if (GetExtension(modelData.meshes[i].DiffuseMap) == TexExtension::PSD) {
				diffusePath = ReplaceExtension(modelData.meshes[i].DiffuseMap, "png"); //psdの場合はtgaとして扱う
			}
		}

		auto normalPath = modelData.meshes[i].NormalMap;
		if (!modelData.meshes[i].NormalMap.empty()) {
			if (GetExtension(modelData.meshes[i].NormalMap) == TexExtension::PSD) {
				normalPath = ReplaceExtension(modelData.meshes[i].NormalMap, "png"); //psdの場合はtgaとして扱う
			}
		}

		TextureSRVData srvData;
		ResultErrorHandle::ErrorCode errorHandle = TextureLoader::GetInstance()->Load(diffusePath);
		auto mainTex = TextureLoader::GetInstance()->Get(diffusePath);
		auto handle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), diffusePath);
		srvData.diffuseMap = handle;

		ResultErrorHandle::ErrorCode normalMapErrorHandle = TextureLoader::GetInstance()->Load(normalPath);
		auto normalTex = TextureLoader::GetInstance()->Get(normalPath);
		auto normalHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), normalPath);
		srvData.normalMap = normalHandle;

		materialHandles.push_back(srvData);
	}

	rootSignature = std::make_unique<RootSignature>();
	rootSignature->SetModelRootSignature();
	if (!rootSignature->IsSuccess())
	{
		OutputDebugStringW(L"ルートシグネチャの生成に失敗\n");
	}

	SetShadowPipelineState();
}

void FBXModel::SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint)
{
	pipelineState = std::make_unique<PipelineState>(topology);
	pipelineState->SetInputLayout(VertexSkinning3D::InputLayout);
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

void FBXModel::SetShadowPipelineState()
{
    shadowRootSignature = std::make_unique<RootSignature>();
    shadowRootSignature->SetShadowDepthRootSignature();
    if (!shadowRootSignature->IsSuccess())
    {
        OutputDebugStringW(L"Shadow用RootSignatureの生成に失敗\n");
        return;
    }

    shadowPipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    shadowPipelineState->SetInputLayout(VertexSkinning3D::InputLayout);
    shadowPipelineState->SetRootSignature(shadowRootSignature->Get());
    shadowPipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    shadowPipelineState->SetVS(L"Shader/FBXModelShader/FBXModelDepthVertexShader.hlsl","DepthVS_FBX");
    shadowPipelineState->CreateDepthOnly();

    if (!shadowPipelineState->IsSuccess())
    {
        OutputDebugStringW(L"Shadow用PipelineStateの生成に失敗\n");
    }
}

void FBXModel::DrawShadow(const ShadowCamera& camera, const ShadowMap& _shadowMap)
{
	shadowMapHandle = _shadowMap.GetSRV();
	if (!ShadowMapCreater::GetInstance()->GetUseShadowMap()) {
		return;
	}
    if (shadowPipelineState == nullptr || !shadowPipelineState->IsSuccess() ||
		constantBufferPtr.shadow.empty() || constantBufferPtr.bone.empty() || modelData.meshes.empty())
    {
        return;
    }

    const UINT currentIndex = Engine::GetInstance()->CurrentBackBufferIndex();
    if (currentIndex >= constantBufferPtr.shadow.size() || currentIndex >= constantBufferPtr.bone.size())
    {
        return;
    }

    auto* commandList = Engine::GetInstance()->CommandList();

    auto* transformData = constantBufferPtr.shadow[currentIndex]->GetPtr<MatrixTransform>();
    transformData->World = BuildWorldMatrix();
    transformData->View = camera.GetViewMatrix();
    transformData->Proj = camera.GetProjectionMatrix();
    transformData->uvRect = DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	Matrix lightViewProjection =camera.GetViewMatrix() * camera.GetProjectionMatrix();

	transformData->lightViewProjection = lightViewProjection;

	auto* constBuff = constantBufferPtr.matrixTransform[currentIndex]->GetPtr<MatrixTransform>();
	constBuff->lightViewProjection = lightViewProjection;

    commandList->SetGraphicsRootSignature(shadowRootSignature->Get());
    commandList->SetPipelineState(shadowPipelineState->GetPipelineState());

    // ShadowDepthRootSignature: RP0=b0 Transform, RP1=b2 BoneBuffer。
    commandList->SetGraphicsRootConstantBufferView(0,constantBufferPtr.shadow[currentIndex]->GetAddress());
    commandList->SetGraphicsRootConstantBufferView(1,constantBufferPtr.bone[currentIndex]->GetAddress());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (size_t i = 0; i < modelData.meshes.size(); ++i)
    {
        if (i >= vertexBuffer.size() || i >= indexBuffer.size())
        {
            continue;
        }

        const auto vbView = vertexBuffer[i]->GetView();
        const auto ibView = indexBuffer[i]->View();

        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->DrawIndexedInstanced(static_cast<UINT>(modelData.meshes[i].Indices.size()),1, 0, 0, 0);
    }
}

void FBXModel::VertexMapping(const Camera3D* _camera)
{
	Vector3 scale = transform.scale;
	Vector3 rotation = transform.rotation;
	Vector3 position = transform.position;

	Matrix world = BuildWorldMatrix();

	Vector4 plane = Vector4(0, 1, 0, 0);
	Vector3 lightVec = Light::GetInstance()->GetLightBufferData().lights[0].direction * -1.0f;
	Matrix shadow = DirectX::XMMatrixShadow(DirectX::XMLoadFloat4(&plane), DirectX::XMLoadFloat3(&lightVec));
	
	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		auto ptr = constantBufferPtr.matrixTransform[i]->GetPtr<MatrixTransform>();
		ptr->World = world;
		ptr->View = _camera->GetViewMatrix();
		ptr->Proj = _camera->GetProjectionMatrix();
		ptr->uvRect = Vector4();
		ptr->shadow = shadow;

		auto light = constantBufferPtr.light[i]->GetPtr<LightBufferData>();
		LightBufferData l = Light::GetInstance()->GetLightBufferData();
		*light = l;

		auto* bone = constantBufferPtr.bone[i]->GetPtr<BoneBuffer>();
		const auto& matrices = animator->GetBoneMatrices();

		const size_t count = std::min(matrices.size(),static_cast<size_t>(BONE_MAX));

		for (size_t j = 0; j < count; ++j)
		{
			DirectX::XMStoreFloat4x4(&bone->bones[j],matrices[j]);
		}

		auto specular = constantBufferPtr.specularMap[i]->GetPtr<SpecularParam>();
		*specular = specularParam;

		auto normal = constantBufferPtr.normalMap[i]->GetPtr<NormalMapParam>();
		*normal = normalMapParam;
	}
}

Matrix FBXModel::BuildWorldMatrix() const
{
	Matrix world = Matrix::Scale(transform.scale) * Matrix::Rotation(transform.rotation) * Matrix::Translation(transform.position);

	// 取り付けられているなら、親のノードの姿勢を掛ける
	if (attachParent)
		world = world * attachParent->GetSocketMatrix(attachNode);

	return world;
}

Matrix FBXModel::GetSocketMatrix(int nodeIndex) const
{
	if (!animator)
		return BuildWorldMatrix();

	const Matrix model = animator->GetNodeModelMatrix(nodeIndex);

	// スケールを取り除く（剣がボーンのスケールで伸び縮みしないように）
	Matrix rigid = model;
	DirectX::XMVECTOR s, r, t;
	if (DirectX::XMMatrixDecompose(&s, &r, &t, model))
	{
		rigid = DirectX::XMMatrixRotationQuaternion(r) * DirectX::XMMatrixTranslationFromVector(t);
	}

	// 親自身のワールド行列（親のスケールは、そのまま武器にも掛かる）
	return rigid * BuildWorldMatrix();
}

void FBXModel::ShaderParamSerializeImgui()
{
	// ShaderEdit ウィンドウに複数のモデルが並ぶので、同じ名前の項目がぶつからないように ID をモデルごとに分ける
	ImGui::PushID(this);

	const std::string title = tag.empty() ? std::string("FBXModel") : tag;

	if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		// ================= ノーマルマップ =================
		ImGui::TextUnformatted(u8"ノーマルマップ");
		ImGui::Separator();

		bool normalOn = normalMapParam.enableNormal > 0.5f;
		if (ImGui::Checkbox(u8"有効##normal", &normalOn))
		{
			normalMapParam.enableNormal = normalOn ? 1.0f : 0.0f;
		}

		ImGui::SliderFloat(u8"強さ##normal", &normalMapParam.normalStrength, 0.0f, 2.0f, "%.2f");

		ImGui::Spacing();

		// ================= スペキュラ =================
		ImGui::TextUnformatted(u8"スペキュラ（ハイライト）");
		ImGui::Separator();

		bool specularOn = specularParam.enableSpecular > 0.5f;
		if (ImGui::Checkbox(u8"有効##specular", &specularOn))
		{
			specularParam.enableSpecular = specularOn ? 1.0f : 0.0f;
		}
		HelpTooltip(u8"OFF にすると、ツヤ（ハイライト）が出なくなります");

		BeginDisabledIf(!specularOn);

		ImGui::SliderFloat(u8"強さ##specular", &specularParam.specularStrength, 0.0f, 2.0f, "%.2f");
		HelpTooltip(u8"ハイライトの明るさ。0.0 で見えなくなります");

		ImGui::SliderFloat(u8"鋭さ", &specularParam.specularPow, 1.0f, 256.0f, "%.1f");
		HelpTooltip(u8"大きいほどハイライトが小さく鋭くなります（金属・つるつる）。小さいと広くぼやけます（ざらざら）");

		ImGui::TextUnformatted(u8"質感の目安:");
		ImGui::SameLine();
		if (ImGui::Button(u8"ざらざら")) { specularParam.specularPow = 8.0f; }
		ImGui::SameLine();
		if (ImGui::Button(u8"普通")) { specularParam.specularPow = 32.0f; }
		ImGui::SameLine();
		if (ImGui::Button(u8"つるつる")) { specularParam.specularPow = 128.0f; }

		// 色は x,y,z (RGB) だけを編集する。w は触らない
		ImGui::ColorEdit3(u8"色", &specularParam.specularColor.x);
		HelpTooltip(u8"ハイライトの色。基本は白。金属っぽくしたいときは素材の色に寄せます");

		EndDisabledIf();

		ImGui::Spacing();

		// ================= リセット =================
		// NormalMapParam / SpecularParam の既定値（構造体で決めている初期値）に戻す
		if (ImGui::Button(u8"初期値に戻す"))
		{
			normalMapParam = NormalMapParam();
			specularParam = SpecularParam();
		}
	}

	ImGui::PopID();
}

void FBXModel::AttachToNode(FBXModel* parent, const std::wstring& nodeName)
{
	Detach();

	if (!parent || parent == this)
		return;

	// 循環（AがBに、BがAに取り付く）を防ぐ
	for (const FBXModel* p = parent; p; p = p->attachParent) {
		if (p == this)
			return;
	}

	const int node = parent->GetNodeIndex(nodeName);
	if (node < 0)
	{
		OutputDebugStringW((L"AttachToNode: ノードが見つかりません: " + nodeName + L"\n").c_str());
		return;
	}

	attachParent = parent;
	attachNode = node;
	parent->attachedChildren.push_back(this);
}

void FBXModel::Detach()
{
	if (attachParent)
	{
		auto& v = attachParent->attachedChildren;
		v.erase(std::remove(v.begin(), v.end(), this), v.end());
	}
	attachParent = nullptr;
	attachNode = -1;
}

void FBXModel::SetBlendMode(BlendState _blendState)
{
	pipelineState->SetBlendMode(_blendState);
}

void FBXModel::SetAlpha(float _alphaValue)
{
	const float alpha = _alphaValue >= 1.0f ? 1.0f : _alphaValue;
	// メッシュごとに頂点カラーを変更
	for (size_t i = 0; i < modelData.meshes.size(); ++i)
	{
		auto& meshVertices = modelData.meshes[i].Vertices;

		for (auto& vertex : meshVertices)
		{
			vertex.color.w = alpha;
		}

		// 対応する頂点バッファへ反映
		if (i < vertexBuffer.size() && vertexBuffer[i])
		{
			vertexBuffer[i]->BufferMapping(meshVertices.data());
		}
	}
}

void FBXModel::SetColor(const Rgb& _rgb)
{
	SetColor(Color::RGBToVector3(_rgb));
}

void FBXModel::SetColor(const Vector3& _color)
{
	// メッシュごとに頂点カラーを変更
	for (size_t i = 0; i < modelData.meshes.size(); ++i)
	{
		auto& meshVertices = modelData.meshes[i].Vertices;

		for (auto& vertex : meshVertices)
		{
			// RGBのみ変更してアルファ値は維持する
			vertex.color.x = _color.x;
			vertex.color.y = _color.y;
			vertex.color.z = _color.z;
		}

		// 対応する頂点バッファへ反映
		if (i < vertexBuffer.size() && vertexBuffer[i])
		{
			vertexBuffer[i]->BufferMapping(meshVertices.data());
		}
	}

	baseColor = _color;
}