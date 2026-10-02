#pragma once
#include "../Object2D/Object2D.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;

class Polygon2D : public Object2D
{
public:
	/// <summary>
	/// ìhÇËÇ¬Ç‘Ç∑Ç©ÇåàÇﬂÇÈ
	/// </summary>
	/// <param name="_fill">ìhÇËÇ¬Ç‘Ç∑èÍçáÇÕtrue</param>
	virtual void SetFill(bool _fill);

	virtual void SetColor(Vector4 _color);

protected:

	void Init(const std::vector<Vertex>& _vertices,const std::vector<uint32_t>& _indices);

	void SetPipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology,std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);
	void SetWireFramePipelineState(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology,std::wstring _VSfilePath, std::string _VSentryPoint, std::wstring _PSfilePath, std::string _PSentryPoint);

    void Set2DMatrix();

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

	virtual void Draw() override;

	bool fillMode;
};