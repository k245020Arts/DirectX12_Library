#pragma once
#include "../Polygon2D .h"

class Box2D : public Polygon2D
{
public:
	Box2D();
	~Box2D();

	void SetLength(const Vector2& _length);

	void SetFill(bool _fill)override;

private:
	void Update()override;
	void Draw()override;

	Vector2 length;
};
