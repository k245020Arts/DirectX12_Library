#pragma once
#include <wrl/client.h>
template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

static constexpr int FRAME_BUFFER_COUNT = 2;