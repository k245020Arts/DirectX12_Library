#pragma once
#include "../Polygon3D.h"

class Sphere3D : public Polygon3D
{
public:
	Sphere3D();
	~Sphere3D();

	struct SphereParam
	{
		float radius = 0;
		uint32_t sliceCount = 0;
		uint32_t stackCount = 0;

		SphereParam()
		{
			radius = 0;
			sliceCount = 0;
			stackCount = 0;
		}

		/// <summary>
		/// 半径のみ設定するコンストラクタ、その他の項目はデフォルト値
		/// </summary>
		/// <param name="_radius">球の半径</param>
		SphereParam(float _radius)
		{
			radius = _radius;
			sliceCount = 32;
			stackCount = 16;
		}

		/// <summary>
		/// 全てのパラメーターを設定するコンストラクタ
		/// </summary>
		/// <param name="_radius">半径</param>
		/// <param name="_sliceCount">横の分割数</param>
		/// <param name="_stackCount">縦の分割数</param>
		SphereParam(float _radius, uint32_t _sliceCount, uint32_t _stackCount)
		{
			radius = _radius;
			sliceCount = _sliceCount;
			stackCount = _stackCount;
		}
	};

	void SetSphereParam(const SphereParam& _param);

	void SetFill(bool _fill)override;

private:

	void Update()override;
	void Draw()override;

	void CreateVertexSphere(float _radius, uint32_t _sliceCount, uint32_t _stackCount, std::vector<uint32_t>& indices);

	void CreateFillIndex(uint32_t _sliceCount, uint32_t _stackCount, std::vector<uint32_t>& indices);
	void CreateGridIndices(uint32_t _sliceCount, uint32_t _stackCount, std::vector<uint32_t>& indices);

	SphereParam param;
};