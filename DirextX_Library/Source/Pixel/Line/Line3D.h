#pragma once
#include "../../ShaderStruct/ShaderStruct.h"
#include "../../ShaderStruct/ShaderStruct.h"
#include "../../Texture/TextureLoader.h"
#include "../../DescriptorHeap/DescriptorHeap.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;

class Line3D
{
public:
	Line3D();
	~Line3D();
	void Update();
	void Draw();

	void Init();

	Transform& GetTransform() { return transform; }

private:

	void VertexMapping();
	void Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);
	void SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

	Vector3 length;

	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<PipelineState> wireFramePipelineState;
	std::unique_ptr<IndexBuffer> indexBuffer;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

	bool useIndex;

	UINT indexSize;
	UINT polygonSize;

	std::vector<Vertex>vertices;

	D3D12_PRIMITIVE_TOPOLOGY drawType;

	bool fillMode;

	Vector4 uvRect;

	TextureData textureData;

	// SRV
	DescriptorHandle* descriptorHandle = nullptr;

	Transform transform;

};