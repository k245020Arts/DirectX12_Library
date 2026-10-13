#pragma once
#include "../SingleTon/SingletonBase.h"
#include <memory>

class DirectX12Imgui;
class Window;

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

};
