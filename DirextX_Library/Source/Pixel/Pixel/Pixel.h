#pragma once
#include "../../ShaderStruct/ShaderStruct.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;

class Pixel
{
public:
	Pixel();
	~Pixel();

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