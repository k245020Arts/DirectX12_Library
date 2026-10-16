#pragma once
#include "MouseCode.h"
#include <Windows.h>
#include "../../Helper/Vector/VectorHelper.h"

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

        Vector2 pos = Vector2();

        int wheelDelta = 0;          // ホイール回転量

        Vector2 deltaPos = Vector2();
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
    float GetMouseX() const { return mouse.pos.x; }
    float GetMouseY() const { return mouse.pos.y; }

    // 座標取得
    float GetDeltaMouseX() const { return mouse.deltaPos.x; }
    float GetDeltaMouseY() const { return mouse.deltaPos.y; }

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
        mouse.pos.x = (float)x;
        mouse.pos.y = (float)y;
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

    Vector2 previewMousePos;

    bool isFirstMove;
};