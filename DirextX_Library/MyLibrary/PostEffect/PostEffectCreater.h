#pragma once
#include "../SingleTon/SingletonBase.h"
#include <memory>

class PostEffectPipeline;
class GaussianBlur;
class BloomEffect;
class DistortionPostEffect;
class FogEffect;
class ShadowMap;
class ShadowCamera;
class ShadowMapDebug;
class RenderPass;
class MotionBlur;
class Vignette;

class PostEffectCreater : public SingletonBase<PostEffectCreater>
{
public:
	PostEffectCreater();
	~PostEffectCreater();

	void Init();
	void Update();

	void EnableGaussainBlur(bool _enable);
	void EnableDistortion(bool _enable);
	void EnableBloom(bool _enable);

private:

	// ポストエフェクト(RenderTarget / RenderPass も内部で自動管理)
	std::unique_ptr<PostEffectPipeline> postEffectPipeline;

	// 個別に操作したいエフェクト(所有権は postEffectPipeline が持つので delete しない)
	BloomEffect* bloom = nullptr;
	GaussianBlur* gaussianBlur = nullptr;
	DistortionPostEffect* distortion = nullptr;
	FogEffect* fog = nullptr;
	MotionBlur* motionBlur =nullptr;
	Vignette* vignette =nullptr;

	friend class SingletonBase<PostEffectCreater>;

};