#pragma once
#include "../SingleTon/SingletonBase.h"
#include <memory>

class PostEffectPipeline;
class GaussianBlur;
class DistortionPostEffect;
class ShadowMap;
class ShadowCamera;
class ShadowMapDebug;
class RenderPass;

class PostEffectCreater : public SingletonBase<PostEffectCreater>
{
public:
	PostEffectCreater();
	~PostEffectCreater();

	void Init();
	void Update();

	void EnableGaussainBlur(bool _enable);
	void EnableDistortion(bool _enable);

private:

	// ポストエフェクト(RenderTarget / RenderPass も内部で自動管理)
	std::unique_ptr<PostEffectPipeline> postEffectPipeline;

	// 個別に操作したいエフェクト(所有権は postEffectPipeline が持つので delete しない)
	GaussianBlur* gaussianBlur = nullptr;
	DistortionPostEffect* distortion = nullptr;

	friend class SingletonBase<PostEffectCreater>;

	bool showShadowMap = true;

};