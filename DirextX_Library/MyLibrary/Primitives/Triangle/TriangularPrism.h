#pragma once
#include "../Polygon3D.h"
#include "../../Texture/TextureLoader.h"
#include "../../DescriptorHeap/DescriptorHeap.h"


class TriangularPrism : public Polygon3D
{
public:
	TriangularPrism();
	~TriangularPrism();

private:
	void Update()override;
	void Draw()override;
};
