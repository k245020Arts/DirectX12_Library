#pragma once
#include "../../ShaderStruct/ShaderStruct.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;

class Line2D
{
public:
	Line2D();
	~Line2D();

	void Update();
	void Draw();

	Transform& GetTransform() { return transform; };

private:
	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

	void SetTriangleMatrix();

	Transform transform;

};