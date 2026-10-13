#pragma once
#include "../Polygon3D.h"

class Cylinder : public Polygon3D
{
public:
	Cylinder();
	~Cylinder();

private:

	void Update()override;
	void Draw()override;

};