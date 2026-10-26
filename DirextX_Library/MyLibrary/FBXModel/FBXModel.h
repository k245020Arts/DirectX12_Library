#pragma once
#include "../assimp/AssimpLoader.h"
#include <memory>
#include "../Object3D/Object3D.h"
#include "../Light/Light.h"
#include <unordered_map>

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;
class Animator;

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

static constexpr int BONE_MAX = 1024;

struct BoneBuffer
{
	DirectX::XMFLOAT4X4 bones[BONE_MAX] = {};
};

class FBXModel : public Object3D
{
public:

	FBXModel();
	~FBXModel();

	bool Load(const std::string& _modelFilePath);
	bool Load(const std::string& _modelFilePath,bool _uflip,bool _vflip);

	bool Load(const wchar_t* _modelFilePath);
	bool Load(const wchar_t* _modelFilePath, bool _uflip, bool _vflip);

	void Update();
	void Draw();

	Transform& GetTransform() { return transform; }

private:

	void Init();

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

	
	ModelData modelData;

	std::unique_ptr<Animator> animator;

	std::vector<std::unique_ptr<VertexBuffer>> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<PipelineState> wireFramePipelineState;
	std::vector <std::unique_ptr<IndexBuffer>> indexBuffer;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;
	std::vector<std::shared_ptr<ConstBuffer>> lightConstBuffer;
	std::vector<std::shared_ptr<ConstBuffer>> boneConstBuffer;

	bool useIndex;

	UINT indexSize;
	UINT polygonSize;

	std::vector<Vertex>vertices;

	D3D12_PRIMITIVE_TOPOLOGY drawType;

	std::vector<DescriptorHandle*> materialHandles; // テクスチャ用のハンドル一覧

	void VertexMapping(const Camera3D* _camera)override;

};
