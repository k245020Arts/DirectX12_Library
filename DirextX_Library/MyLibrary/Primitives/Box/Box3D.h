#pragma once
#include "../Polygon3D.h"

class Box3D : public Polygon3D
{
public:
	Box3D();
	~Box3D();

	void SetLength(const Vector3& _length);

private:

	void Update()override;
	void Draw()override;

	void CreateVertexPosition();

	Vector3 length;
};