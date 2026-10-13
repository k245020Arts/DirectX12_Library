#pragma once
#include "../Polygon3D.h"

class Capsule : public Polygon3D
{
public:
	Capsule();
	~Capsule();

private:

	void Update()override;
	void Draw()override;

};