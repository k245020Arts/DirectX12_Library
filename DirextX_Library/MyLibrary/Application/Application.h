#pragma once
#include "../SingleTon/SingletonBase.h"
#include <memory>

class DirectX12Imgui;
class Window;
class RenderTarget;
class RenderPass;
class PostEffect;
class GaussianBlur;
class DistortionPostEffect;

class Application : public SingletonBase<Application>
{
public:
	
private:
	Application();
	~Application();

	void ApplicationInit(Window window);

	void BeginUpdate();
	void ApplicationUpdate();
	void ApplicationDraw();

	void ApplicationRelease();

	friend class Main;
	friend class SingletonBase<Application>;

	std::unique_ptr<DirectX12Imgui> imgui;

	std::shared_ptr<RenderTarget> sceneRenderTarget;
	std::shared_ptr<RenderTarget> blurRenderTarget;

	std::shared_ptr<RenderPass> scenePass;
	std::shared_ptr<RenderPass> horizontalBlurPass;
	std::shared_ptr<RenderPass> verticalBlurPass;
	std::shared_ptr<RenderPass> distortionPass;

	std::unique_ptr<GaussianBlur> gaussianBlur;
	std::unique_ptr<DistortionPostEffect> distortion;

};
