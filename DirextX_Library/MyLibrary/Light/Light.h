#pragma once
#include "../Helper/Helper.h"
#include "../SingleTon/SingletonBase.h"

struct LightData
{
    Vector3 direction;
    float intensity;

    Vector3 color;
    float padding; //上みたいに16バイトごとに色を管理することによって、hlslにデータを渡しやすくなる

    LightData()
    {
        direction = Vector3(1.0f, -1.0f, 1.0f);
        direction.Normalize();
        intensity = 1.0f;
        color = VectorHelper::VOne;
        padding = 0.0f;
    }

    LightData(const Vector3& _direction, float _intensity, const Vector3& _color)
    {
        direction = _direction;
        direction.Normalize();
        intensity = _intensity;
        color = _color;
        padding = 0.0f;
    }
};

constexpr uint32_t MAX_LIGHT_COUNT = 12u;

struct LightBufferData
{
    LightData lights[MAX_LIGHT_COUNT]; //固定長配列の方がGPUにデータを渡しやすいためあえて使っている。
    uint32_t lightCount;
    float padding[3];

    // アンビエントライト
    Vector3 ambientColor;
    float ambientIntensity;

    LightBufferData()
    {
        lightCount = 0;

        ambientColor = VectorHelper::VOne;
        ambientIntensity = 0.2f;
    }
};

class Light : public SingletonBase<Light>
{
public:
    Light();
    ~Light();
    void Update();

    void SetDirection(uint32_t _lightIndex,const Vector3& direction);
    void SetColor(uint32_t _lightIndex, const Vector3& color);
    void SetIntensity(uint32_t _lightIndex, float intensity);

    void SetAmbientColor(const Vector3& color);
    void SetAmbientIntensity(float intensity);

    const LightBufferData& GetLightBufferData()const { return lightBufferData; }

    bool RemoveLight(uint32_t index);

    bool AddLight(const LightData& data);
    bool SetLightData(uint32_t index, const LightData& data);

private:
    LightBufferData lightBufferData{};

    friend class SingletonBase<Light>;
};
