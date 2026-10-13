#pragma once
#include "../Polygon3D.h"

class Sphere3D : public Polygon3D
{
public:
	Sphere3D();
	~Sphere3D();

private:

	void Update()override;
	void Draw()override;

};