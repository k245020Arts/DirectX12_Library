#pragma once
#include "../Polygon2D .h"

class Star2D : public Polygon2D
{
public:
	Star2D();
	~Star2D();

	void SetRadius(float _radius);

	void SetFill(bool _fill)override;

private:

	void Update()override;
	void Draw()override;

	std::vector<Vertex> CreateStarVertices(float _centerX, float _centerY, float _radius, DirectX::XMFLOAT4 _color);

	float radius;
};