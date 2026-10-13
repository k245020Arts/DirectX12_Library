#pragma once
#include "../Polygon2D .h"

struct alignas(256) CicleWireFrame
{
	float wireFrame;
};

class Cicle2D : public Polygon2D
{
public:
	Cicle2D();
	~Cicle2D();

	void SetRadius(float _radius);

	void SetFill(bool _fill)override;

	void FillConstShaderUpdate();

private:
	void Update()override;
	void Draw()override;

	float radius;

	std::vector<std::shared_ptr<ConstBuffer>> cicleWireFrameConstBuffer;

	bool fill;
};