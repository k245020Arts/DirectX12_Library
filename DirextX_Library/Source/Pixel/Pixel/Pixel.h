#pragma once
#include "../Polygon2D .h"

class Pixel : public Polygon2D
{
public:
	Pixel();
	~Pixel();

private:
	void Update()override;
	void Draw()override;
};
