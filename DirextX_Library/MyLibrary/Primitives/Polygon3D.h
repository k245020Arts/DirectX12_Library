#pragma once
#include "../Object3D/Object3D.h"
#include "../ShaderStruct/ShaderStruct.h"
#include <memory>
#include "../Texture/TextureLoader.h"
#include "../DescriptorHeap/DescriptorHeap.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;


class Polygon3D : public Object3D
{
public:
	Polygon3D();
	~Polygon3D();

	/// <summary>
	/// ìhÇËÇ¬Ç‘Ç∑Ç©ÇåàÇﬂÇÈ
	/// </summary>
	/// <param name="_fill">ìhÇËÇ¬Ç‘Ç∑èÍçáÇÕtrue</param>
	virtual void SetFill(bool _fill);

	virtual void SetColor(const Vector4& _color);

	void SetBlendMode(BlendState _blendState);

	void SetAlpha(float _alphaValue);

protected:

	void Update()override;
	void Draw()override;

	void CreateWhiteTextureHandle();

	void VertexMapping(const Camera3D* _camera)override;
	void Init(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);
	void SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology, std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

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

};