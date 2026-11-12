#pragma once
#include "../Helper/Helper.h"
#include "../SingleTon/SingletonBase.h"

class ShadowMap;
class ShadowCamera;
class ShadowMapDebug;
class RenderPass;

class ShadowMapCreater : public SingletonBase<ShadowMapCreater>
{
public:
	ShadowMapCreater();
	~ShadowMapCreater();

	void Update();
	void Init();

	void DebugDrawInit();

	void ShadowCameraDirection(const Vector3& _direction);
	void SetUseShadowMap(const bool _use,const int _w, const int _h);

	bool GetUseShadowMap() { return useShadowMap; }

	void SetShadowTarget(const Vector3& _target);
	void SetFollowCamera(bool _follow) { followCamera = _follow; }

private:

	friend class SingletonBase<ShadowMapCreater>;

	bool showShadowMap = true;
	bool shadowMapCreated = false;
	bool useShadowMap = false;

	std::unique_ptr<ShadowMap> shadowMap;
	std::unique_ptr<ShadowCamera> shadowCamera;
	std::unique_ptr<ShadowMapDebug> debugShadow;
	std::shared_ptr<RenderPass> shadowRenderPass;
	std::shared_ptr<RenderPass> debugShadowPass;
	bool followCamera = false;
};