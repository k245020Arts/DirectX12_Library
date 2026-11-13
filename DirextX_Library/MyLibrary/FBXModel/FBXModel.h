#pragma once
#include "../assimp/AssimpLoader.h"
#include <memory>
#include "../Object3D/Object3D.h"
#include "../Light/Light.h"
#include <unordered_map>
#include "../Shadow/ShadowMap.h"
#include "../ConstBuffer/ConstBuffer.h"

class VertexBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;
class Animator;
class ShadowCamera;

struct IKChain;

struct DescriptorHandle;

struct NodeData
{
	std::wstring name;

	DirectX::XMFLOAT4X4 localTransform{};

	int parentIndex = -1;
	std::vector<uint32_t> children;

	// このノードに所属するメッシュのインデックス
	std::vector<uint32_t> meshIndices;
};

struct ModelData
{
	std::vector<Mesh> meshes;
	std::vector<AnimationData> animations;

	// スキニング用
	std::vector<BoneInfo> bones;
	std::unordered_map<std::wstring, uint32_t> boneMap;

	// アニメーション用のノード階層
	std::vector<NodeData> nodes;
	int rootNodeIndex = -1;

	// ルートノードの逆行列
	Matrix globalInverseTransform;
};

struct TextureSRVData
{
	DescriptorHandle* diffuseMap;
	DescriptorHandle* normalMap;

};

// IKソルバーの種類
enum class IKSolverType
{
	TwoBone,	// 余弦定理。腕・脚向け（ノードをちょうど3つ指定する）
	CCD,		// 反復法。首・背骨・尻尾など、2関節以上の鎖向け
};

class FBXModel : public Object3D
{
public:

	FBXModel();
	~FBXModel();

	ResultErrorHandle Load(const std::string& _modelFilePath);
	ResultErrorHandle Load(const std::string& _modelFilePath,bool _uflip,bool _vflip);

	ResultErrorHandle Load(const wchar_t* _modelFilePath);
	ResultErrorHandle Load(const wchar_t* _modelFilePath, bool _uflip, bool _vflip);

	ResultErrorHandle LoadAnimation(const std::string& _animFilePath);
	ResultErrorHandle LoadAnimation(const std::wstring& _animFilePath);

	void Update();
	void Draw();

	Transform& GetTransform() { return transform; }

	void PlayAnimation(int animIndex, bool loop = true, float blendSeconds = 0.2f);
	void StopAnimation();
	void SetAnimationSpeed(float s);

	int GetAnimationIndex();
	bool GetAnimationPlaying();

	// ---- ノード情報（ワールド空間）----
	// 直近の Update の結果。他のモデルのノードを目標にするなら、そのモデルの Update を先に済ませておくこと
	Matrix  GetNodeWorldMatrix(const std::wstring& nodeName) const;
	Vector3 GetNodeWorldPosition(const std::wstring& nodeName) const;
	int     GetNodeIndex(const std::wstring& nodeName) const;
	const std::wstring& GetNodeName(int nodeIndex) const;
	int     GetNodeCount() const;

	// ---------------- IK ----------------
	// nodeNames: 付け根 → 先端 の順のノード名（TwoBone は3つ、CCD は2つ以上）
	// 戻り値は IK の ID。失敗したら -1
	int  AddIK(const std::wstring& name, IKSolverType type, const std::vector<std::wstring>& nodeNames);
	int  FindIK(const std::wstring& name) const;

	// 目標をワールド座標で指定する。weight: 0 = FKのまま、1 = 完全にIK
	void SetIKTarget(int ikId, const Vector3& worldPos, float weight);
	// 別のノード（IKボーンなど）の位置を目標にする（MMDのIKボーン方式）
	void SetIKTargetNode(int ikId, const std::wstring& nodeName, float weight);
	// 肘・膝が出る側の目安（TwoBone のみ）
	void SetIKPole(int ikId, const Vector3& worldPos);
	void SetIKPoleNode(int ikId, const std::wstring& nodeName);
	void ClearIKPole(int ikId);
	// linkIndex 番目の関節を蝶番にする（軸はボーンのローカル軸。角度はバインド姿勢からの値で、ラジアン）
	void SetIKHinge(int ikId, int linkIndex, const Vector3& boneLocalAxis, float minAngle, float maxAngle);
	void SetIKWeight(int ikId, float weight);
	void SetIKEnabled(int ikId, bool enabled);
	// 蝶番の現在の角度（ラジアン）。可動範囲を決めるときの確認用
	float GetIKHingeAngle(int ikId, int linkIndex) const;
	// CCD の iterations など、細かい設定が必要なとき（Animator.h の IKChain を参照）
	IKChain* GetIK(int ikId);

	// 別のFBXModelのノード（手など）に取り付ける。
	// このモデルの transform は、そのノードから見たオフセットになる
	void AttachToNode(FBXModel* parent, const std::wstring& nodeName);
	void Detach();

	void SetColor(const Vector3& _color);
	void SetColor(const Rgb& _rgb);

	void SetBlendMode(BlendState _blendState);

	void SetAlpha(float _alphaValue);

	void SetGroundShadow(bool _shadow) { groundShadow = _shadow; }

	/// <summary>
	/// ノーマルマップを有効にするかどうか
	/// </summary>
	/// <param name="_enableNormal"></param>
	void EnableToNormalMap(const bool _enableNormal) { normalMapParam.enableNormal = _enableNormal ? 1.0f : 0.0f; }
	void NormalMapStrength(const float _strength) { normalMapParam.normalStrength = _strength; }

	/// <summary>
	/// スペキュラマップを有効にするかどうか
	/// </summary>
	/// <param name="_enableSpecular"></param>
	void EnableToSpecularMap(const bool _enableSpecular) { specularParam.enableSpecular = _enableSpecular ? 1.0f : 0.0f; }
	void SpecularMapStrength(const float _strength) { specularParam.specularStrength = _strength; }
	void SpecularMapPow(const float _pow) { specularParam.specularPow = _pow; }
	void SpecularMapColor(const Rgb& _rgb) { specularParam.specularColor = Color::RGBToVector4(_rgb); }
	void SpecularMapColor(const Vector4& _rgb) { specularParam.specularColor = _rgb; }

private:

    using ConstantPtr = std::vector<std::shared_ptr<ConstBuffer>>;

	struct ConstantBufferData
	{
		ConstantPtr matrixTransform;
		ConstantPtr light;
		ConstantPtr bone;
		ConstantPtr shadow;
		ConstantPtr normalMap;
		ConstantPtr specularMap;

		ConstantBufferData()
		{
			matrixTransform.resize(FRAME_BUFFER_COUNT);
			light.resize(FRAME_BUFFER_COUNT);
			bone.resize(FRAME_BUFFER_COUNT);
			shadow.resize(FRAME_BUFFER_COUNT);
			specularMap.resize(FRAME_BUFFER_COUNT);
			normalMap.resize(FRAME_BUFFER_COUNT);
		}
	};

	/// <summary>
	/// 定数バッファの作成時に使う関数
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="_data"></param>
	/// <param name="_message"></param>
	/// <returns></returns>
	template<typename T>
	bool CreateConstantBufferData(std::shared_ptr<ConstBuffer>& _data,const std::wstring& _message)
	{
		_data = std::make_shared<ConstBuffer>(sizeof(T));
		if (!_data->IsValid())
		{
			OutputDebugStringW(_message.c_str());
			return false;
		}
		return true;
	}

	void Init();

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

	
	void DrawShadow(const ShadowCamera& _camera,const ShadowMap& _shadowMap) override;
	void SetShadowPipelineState();
	ModelData modelData;

	std::unique_ptr<Animator> animator;

	std::vector<std::unique_ptr<VertexBuffer>> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<PipelineState> wireFramePipelineState;
	std::vector <std::unique_ptr<IndexBuffer>> indexBuffer;

	ConstantBufferData constantBufferPtr;
	std::unique_ptr<RootSignature> shadowRootSignature;
	std::unique_ptr<PipelineState> shadowPipelineState;

	bool useIndex;

	UINT indexSize;
	UINT polygonSize;

	std::vector<Vertex>vertices;

	D3D12_PRIMITIVE_TOPOLOGY drawType;

	std::vector<TextureSRVData> materialHandles; // テクスチャ用のハンドル一覧

	void VertexMapping(const Camera3D* _camera)override;

	// 取り付けも含めた、ワールド行列
	Matrix BuildWorldMatrix() const;
	// ノードのワールド空間での姿勢（スケールなし）
	Matrix GetSocketMatrix(int nodeIndex) const;

	FBXModel* attachParent = nullptr;
	int       attachNode = -1;
	std::vector<FBXModel*> attachedChildren;     // 自分に取り付いているモデル

	Vector3 baseColor;

	bool groundShadow = false;
	DescriptorHandle* shadowMapHandle = nullptr;

	NormalMapParam normalMapParam;
	SpecularParam specularParam;

	void ShaderParamSerializeImgui()override;
};
