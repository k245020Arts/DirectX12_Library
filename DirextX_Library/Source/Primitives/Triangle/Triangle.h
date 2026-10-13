#pragma once
#include "../Polygon2D .h"

class Triangle : public Polygon2D
{
public:
	Triangle();
	~Triangle();

private:
	void Update()override;
	void Draw()override;
};
