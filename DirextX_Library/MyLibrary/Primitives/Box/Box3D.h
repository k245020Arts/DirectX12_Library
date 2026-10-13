#pragma once
#include "../Polygon3D.h"

class Box3D : public Polygon3D
{
public:
	Box3D();
	~Box3D();

private:

	void Update()override;
	void Draw()override;

};