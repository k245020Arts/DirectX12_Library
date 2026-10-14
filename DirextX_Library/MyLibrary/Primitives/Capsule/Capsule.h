#pragma once
#include "../Polygon3D.h"

class Capsule : public Polygon3D
{
public:
	Capsule();
	~Capsule();

	struct CapsuleParam
	{
		int slices = 24;             // ‰~ü•ûŒü‚Ì•ªŠ„”
		int stacks = 8;              // ”¼‹…•”•ª‚Ì•ªŠ„”
		float radius = 30.0f;        // ”¼Œa
		float cylinderHeight = 60.0f; // ’†‰›‚Ì‰~’Œ•”•ª‚Ì‚‚³
	};

	void SetCapsuleParam();

private:

	void Update()override;
	void Draw()override;

	void CreateCausulePosition(const int STACKS, const int SLICES, const float RADIUS, const float HALF_CYL_HEIGHT, int& vIndex);

};