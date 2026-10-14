#pragma once
#include "../Polygon3D.h"

class Cylinder : public Polygon3D
{
public:
	Cylinder();
	~Cylinder();

	struct CylinderParam
	{
		float radius = 0.0f;
		float height = 0.0f;
		int slices = 0; // 円周の分割数 : 増やすほど滑らかな円柱

		CylinderParam()
		{
			slices = 0;
			radius = 0.0f;
			height = 0.0f;
		}

		/// <summary>
		/// シリンダーの高さと半径を設定するコンストラクタ
		/// </summary>
		/// <param name="_radius">半径</param>
		/// <param name="_height">高さ</param>
		CylinderParam(float _radius,float _height)
		{
			slices = 24;
			radius = _radius;
			height = _height;
		}

		/// <summary>
		/// シリンダーに必要な情報を全て設定するコンストラクタ
		/// </summary>
		/// <param name="_radius">半径</param>
		/// <param name="_height">高さ</param>
		/// <param name="_slices">分割数、数が多いほど滑らかに</param>
		CylinderParam(float _radius, float _height,int _slices)
		{
			slices = _slices;
			radius = _radius;
			height = _height;
		}
	};

	void SetCylinderParam(const CylinderParam& _param);

private:

	CylinderParam param;

	void Update()override;
	void Draw()override;

	void CreateVertexCylider(int _slices,float _radius,float _height, std::vector<uint32_t>& indices);

};