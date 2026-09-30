#pragma once
#include "../../ShaderStruct/ShaderStruct.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;

class Box2D
{
public:
	Box2D();
	~Box2D();

	void Update();
	void Draw();

	Transform& GetTransform() { return transform; };

private:
	std::unique_ptr<VertexBuffer> vertexBuffer;
	std::unique_ptr<RootSignature> rootSignature;
	std::unique_ptr<PipelineState> pipelineState;
	std::unique_ptr<IndexBuffer> indexBuffer;

	std::vector<std::shared_ptr<ConstBuffer>> constBuffer;

	void SetTriangleMatrix();

	Transform transform;
};
