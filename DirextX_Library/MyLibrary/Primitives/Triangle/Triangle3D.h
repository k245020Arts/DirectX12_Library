#pragma once
#include "../Polygon3D.h"

class VertexBuffer;
class ConstBuffer;
class PipelineState;
class RootSignature;
class IndexBuffer;

class Triangle3D : public Polygon3D
{
public:
	Triangle3D();
	~Triangle3D();

private:

	void Update()override;
	void Draw()override;

	Vector3 length;

};
