#pragma once
#include "../Polygon2D .h"

class Line2D : public Polygon2D
{
public:
	Line2D();
	~Line2D();

	void SetLength(const float _length);

private:
	void Update()override;
	void Draw()override;

	float length;
};
