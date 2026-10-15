#pragma once
#include "../SingleTon/SingletonBase.h"
#include <memory>
#include "Key/KeyInput.h"
#include "Mouse/MouseInput.h"
#include "../ShaderStruct/ShaderStruct.h"	

class InputManager : public SingletonBase<InputManager>
{
public:
	
	bool GetKeyInput(KeyCode vkCode) { return keyInput->IsKeyPress(vkCode); }
	bool GetKeyInputDown(KeyCode vkCode) { return keyInput->IsKeyTrigger(vkCode); }
	bool GetKeyInputUp(KeyCode vkCode) { return keyInput->IsKeyRelease(vkCode); }

	bool GetMouseInput(MouseCode mouseID) { return mouseInput->IsMousePress(mouseID); }
	bool GetMouseInputDown(MouseCode mouseID) { return mouseInput->IsMouseTrigger(mouseID); }
	bool GetMouseInputUp(MouseCode mouseID) { return mouseInput->IsMouseRelease(mouseID); };

	void SetMousePosition(int x, int y) { mouseInput->SetMousePosition(x, y); }
	Vector2 GetMousePosition() { return Vector2((float)mouseInput->GetMouseX(), (float)mouseInput->GetMouseY()); }
	
	void SetWheelDelta(int _wheel) { mouseInput->SetWheelDelta(_wheel); };
	int GetMouseWheelDelta() { return mouseInput->GetWheelDelta(); };

	void SetMouseButtonState(int button, bool isPressed) { mouseInput->SetMouseButtonState(button, isPressed); }

	void SetCurrentKeyCode(int vkCode, bool isPressed) { keyInput->SetKeyState(vkCode, isPressed); };
	void SetMouseDraw(bool _draw) { mouseInput->SetMouseDraw(_draw); }

private:

	InputManager();
	~InputManager();

	void Update();

	friend class SingletonBase<InputManager>;
	friend class Application;

	std::unique_ptr<KeyInput> keyInput;
	std::unique_ptr<MouseInput> mouseInput;

	void SetMouseHwnd(HWND _hwnd) { mouseInput->SetHwnd(_hwnd); }
};
