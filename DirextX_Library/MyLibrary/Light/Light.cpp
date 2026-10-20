#include "Light.h"

Light::Light()
{
    AddLight(LightData()); //初期化時にデフォルトライトを一つ設定
}

Light::~Light()
{
}

void Light::Update()
{

}

// ライトの追加
bool Light::AddLight(const LightData& data)
{
    // 最大数を超えている場合は追加しない
    if (lightBufferData.lightCount >= MAX_LIGHT_COUNT)
    {
        return false;
    }

    // 配列の末尾に追加
    uint32_t index = lightBufferData.lightCount;
    lightBufferData.lights[index] = data;

    // 登録数を更新
    lightBufferData.lightCount++;

    return true;
}

// ライトの変更
bool Light::SetLightData(uint32_t index,const LightData& data)
{
    // 存在しないインデックスは変更しない
    if (index >= lightBufferData.lightCount)
    {
        return false;
    }

    lightBufferData.lights[index] = data;

    return true;
}

// ライトの削除
bool Light::RemoveLight(uint32_t index)
{
    // 存在しないインデックスは削除しない
    if (index >= lightBufferData.lightCount)
    {
        return false;
    }

    // 削除した要素より後ろを1つずつ前に詰める
    for (uint32_t i = index; i + 1 < lightBufferData.lightCount; ++i)
    {
        lightBufferData.lights[i] = lightBufferData.lights[i + 1];
    }

    // 最後の要素を初期化
    lightBufferData.lights[lightBufferData.lightCount - 1] = LightData{};

    // 登録数を減らす
    lightBufferData.lightCount--;

    return true;
}

void Light::SetDirection(uint32_t _lightIndex, const Vector3& direction)
{
    // 存在しないインデックスは変更しない
    if (_lightIndex >= lightBufferData.lightCount)
    {
        return;
    }

	lightBufferData.lights[_lightIndex].direction = direction;
}

void Light::SetColor(uint32_t _lightIndex, const Vector3& color)
{
    // 存在しないインデックスは変更しない
    if (_lightIndex >= lightBufferData.lightCount)
    {
        return;
    }

	lightBufferData.lights[_lightIndex].color = color;
}

void Light::SetIntensity(uint32_t _lightIndex, float intensity)
{
    // 存在しないインデックスは変更しない
    if (_lightIndex >= lightBufferData.lightCount)
    {
        return;
    }

	lightBufferData.lights[_lightIndex].intensity = intensity;
}

void Light::SetAmbientColor(const Vector3& color)
{
	lightBufferData.ambientColor = color;
}

void Light::SetAmbientIntensity(float intensity)
{
	lightBufferData.ambientIntensity = intensity;
}
