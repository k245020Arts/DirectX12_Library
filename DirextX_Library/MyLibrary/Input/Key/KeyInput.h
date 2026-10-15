#pragma once
#include "KeyCode.h"

/// <summary>
/// キーボードのインプットのクラス。基本はInputManagerを介して、実行をするものなので、外側で生成しても何もできないクラスになる
/// </summary>
class KeyInput
{
public:
	KeyInput();
	~KeyInput();

private:

	//押しっぱなし
	bool IsKeyPress(KeyCode keyID) { return g_Input.current[static_cast<int>(keyID)]; }
	//押した瞬間
	bool IsKeyTrigger(KeyCode keyID) { return g_Input.current[static_cast<int>(keyID)] && !g_Input.prev[static_cast<int>(keyID)]; }
	//話した瞬間
	bool IsKeyRelease(KeyCode keyID) { return !g_Input.current[static_cast<int>(keyID)] && g_Input.prev[static_cast<int>(keyID)]; }

	//外部WndProc）からキー状態を設定するための関数
	void SetKeyState(int vkCode, bool isPressed) {
		if (vkCode >= 0 && vkCode < 256) {
			g_Input.current[vkCode] = isPressed;
		}
	}

	void UpdateInputState();

	// 入力状態を管理する構造体の例
	struct Input {
		bool current[256] = { false }; // 現在のフレームの押下状態
		bool prev[256] = { false }; // 1フレーム前の押下状態
	};
	Input g_Input;

	friend class InputManager;
};