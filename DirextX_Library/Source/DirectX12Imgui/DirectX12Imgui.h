#pragma once
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../../ImGui/imgui_impl_dx12.h"
#include "../DescriptorHeap/DescriptorHeap.h"

class DirectX12Imgui
{
public:
	DirectX12Imgui();
	~DirectX12Imgui();

	

private:
	DescriptorHandle* imguiHandle = nullptr;

	void SetUpImGui(HWND hwnd);

	void BeginRenderImGui();

	void EndRenderImGui();

	void ReleaseImGui();

	void DebugRenderer();

	friend class Main;
};
