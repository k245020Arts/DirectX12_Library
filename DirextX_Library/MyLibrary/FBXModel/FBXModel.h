#pragma once
#include "../assimp/AssimpLoader.h"
#include <memory>
#include "../Object3D/Object3D.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;

struct DescriptorHandle;

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

	std::vector<Mesh> meshes;

	std::vector<std::unique_ptr<VertexBuffer>> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<PipelineState> wireFramePipelineState;
	std::vector <std::unique_ptr<IndexBuffer>> indexBuffer;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

	bool useIndex;

	UINT indexSize;
	UINT polygonSize;

	std::vector<Vertex>vertices;

	D3D12_PRIMITIVE_TOPOLOGY drawType;

	std::vector<DescriptorHandle*> materialHandles; // テクスチャ用のハンドル一覧

	void VertexMapping(const Camera3D* _camera)override;

};
