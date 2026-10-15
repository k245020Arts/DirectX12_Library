#include "KeyInput.h"

KeyInput::KeyInput()
{
}

KeyInput::~KeyInput()
{
}

void KeyInput::UpdateInputState()
{
    // 現在の状態を1フレーム前の状態としてコピー
    for (int i = 0; i < 256; ++i) {
        g_Input.prev[i] = g_Input.current[i];
    }
}
