#pragma once
#include "MouseCode.h"
#include <Windows.h>

/// <summary>
/// マウス入力を管理するクラス。
/// 基本はInputManagerを介して使用する。
/// </summary>
class MouseInput
{
public:
    MouseInput();
    ~MouseInput();

private:

    void SetHwnd(HWND _hwnd) { hwnd = _hwnd; }
    // マウスボタンの状態
    struct MouseState
    {
        bool current[3] = { false }; // 現在のフレーム
        bool prev[3] = { false };    // 1フレーム前

        int x = 0;                   // マウス座標X
        int y = 0;                   // マウス座標Y

        int wheelDelta = 0;          // ホイール回転量
    };

    MouseState mouse;

    // 押しっぱなし
    bool IsMousePress(MouseCode mouseID)
    {
        return mouse.current[static_cast<int>(mouseID)];
    }

    // 押した瞬間
    bool IsMouseTrigger(MouseCode mouseID)
    {
        return mouse.current[static_cast<int>(mouseID)] && !mouse.prev[static_cast<int>(mouseID)];
    }

    // 離した瞬間
    bool IsMouseRelease(MouseCode mouseID)
    {
        return !mouse.current[static_cast<int>(mouseID)] && mouse.prev[static_cast<int>(mouseID)];
    }

    // 座標取得
    int GetMouseX() const { return mouse.x; }
    int GetMouseY() const { return mouse.y; }

    // ホイール回転量取得
    int GetWheelDelta() const { return mouse.wheelDelta; }

    // 外部（WndProc）から状態を設定する関数
    void SetMouseButtonState(int button, bool isPressed)
    {
        if (button >= 0 && button < 3)
        {
            mouse.current[button] = isPressed;
        }
    }

    void SetMousePosition(int x, int y)
    {
        mouse.x = x;
        mouse.y = y;
    }

    void SetWheelDelta(int delta)
    {
        mouse.wheelDelta += delta;
    }

    void SetMouseDraw(bool _draw)
    {
        ShowCursor(_draw);
    }

    void UpdateInputState();

    friend class InputManager;

    HWND hwnd;
};