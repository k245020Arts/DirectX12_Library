#pragma once
#include "../Polygon3D.h"
#include "../../Texture/TextureLoader.h"
#include "../../DescriptorHeap/DescriptorHeap.h"


class TriangularPrism : public Polygon3D
{
public:
	TriangularPrism();
	~TriangularPrism();

	void SetLength(const Vector3& _length);

	void SetFill(bool _fill)override;

private:
	void Update()override;
	void Draw()override;

	void SetVertices(const Vector3& _length);

	Vector3 length;
};
