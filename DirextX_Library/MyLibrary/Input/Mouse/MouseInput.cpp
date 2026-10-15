#include "MouseInput.h"

MouseInput::MouseInput()
{
}

MouseInput::~MouseInput()
{
}

void MouseInput::UpdateInputState()
{
    for (int i = 0; i < 3; ++i)
    {
        mouse.prev[i] = mouse.current[i];
    }

    // ホイールはフレームごとの変化量として扱う
    mouse.wheelDelta = 0;

    //座標更新
    POINT pos;

    if (GetCursorPos(&pos))
    {
        ScreenToClient(hwnd, &pos);

        mouse.x = pos.x;
        mouse.y = pos.y;
    }
}