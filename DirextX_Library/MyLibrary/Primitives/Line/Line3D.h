#pragma once
#include "../Polygon3D.h"

class Line3D : public Polygon3D
{
public:
	Line3D();
	~Line3D();

	void SetLength(const Vector3& _length);

	void SetStartAndEnd(const Vector3& _start, const Vector3& _end);

private:

	void Update()override;
	void Draw()override;

	Vector3 length;
};