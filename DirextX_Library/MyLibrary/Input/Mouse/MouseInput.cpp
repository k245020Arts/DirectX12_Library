#include "MouseInput.h"
#include "../../ShaderStruct/ShaderStruct.h"

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

        mouse.pos.x = (float)pos.x;
        mouse.pos.y = (float)pos.y;
    }

    const Vector2 current = mouse.pos;

    mouse.deltaPos = current - previewMousePos;

    previewMousePos = current;

}