#pragma once
#include "../Polygon3D.h"

class Line3D : public Polygon3D
{
public:
	Line3D();
	~Line3D();

private:

	void Update()override;
	void Draw()override;

};