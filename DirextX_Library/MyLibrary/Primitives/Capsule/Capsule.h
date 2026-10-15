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

		CapsuleParam()
		{
			slices = 0;
			stacks = 0;
			radius = 0.f;
			cylinderHeight = 0.f;
		}

		/// <summary>
		/// ƒRƒ“ƒXƒgƒ‰ƒNƒ^
		/// </summary>
		/// <param name="_slices">‰~ü•ûŒü‚Ì•ªŠ„”</param>
		/// <param name="_stacks">”¼‹…•”•ª‚Ì•ªŠ„”</param>
		/// <param name="_radius">”¼Œa</param>
		/// <param name="_cylinderHeight">’†‰›‚Ì‰~’Œ•”•ª‚Ì‚‚³</param>
		CapsuleParam(int _slices,int _stacks,float _radius,float _cylinderHeight)
		{
			slices = _slices;
			stacks = _stacks;
			radius = _radius;
			cylinderHeight = _cylinderHeight;
		}
	};

	void SetCapsuleParam(const CapsuleParam& _param);

	void SetFill(bool _fill)override;

private:

	void Update()override;
	void Draw()override;

	void CreateCausulePosition(const int _stacks, const int _slices, const float _radius, const float _height, std::vector<uint32_t>& indices);

	void CreateFillIndices(const int _stacks, const int _slices, std::vector<uint32_t>& indices);
	void CreateWireFrameIndices(const int _stacks, const int _slices, std::vector<uint32_t>& indices);

	CapsuleParam capsuleParam;
};