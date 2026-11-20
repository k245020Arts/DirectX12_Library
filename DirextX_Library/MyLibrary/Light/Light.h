#pragma once
#include "../Helper/Helper.h"
#include "../SingleTon/SingletonBase.h"

enum class LightType : uint32_t
{
    No_Init = 0,
    Directional = 1,
    Point = 2,
    Spot = 3
};

struct LightData
{
    
    Vector3 direction;
    float intensity;

    Vector3 color;
    uint32_t lightType;
    
    Vector3 position;
    float range;

    float innerCos;      // スポットの内側角度の cos (これより内側は全開)
    float outerCos;      // スポットの外側角度の cos (これより外側は0)
    float pad[2];

    LightData()
    {
        direction = Vector3(1.0f, -1.0f, 1.0f);
        direction.Normalize();
        intensity = 1.0f;
        color = VectorHelper::VOne;
        lightType = static_cast<uint32_t>(LightType::Directional);
        position = Vector3(0.0f, 0.0f, 0.0f);
        range = 10.0f;
        innerCos = 1.0f;
        outerCos = 0.0f;
        pad[0] = pad[1] = 0.0f;
    }
    /// <summary>
    /// ディレクショナルライトの設定用の関数
    /// </summary>
    /// <param name="_direction">向き</param>
    /// <param name="_intensity">強さ</param>
    /// <param name="_color">色</param>
    /// <returns></returns>
    static LightData SetDirectionalLightData(const Vector3& _direction, float _intensity, const Vector3& _color)
    {
        LightData data;
        data.lightType = static_cast<uint32_t>(LightType::Directional);
        data.direction = _direction;
        data.direction.Normalize();
        data.intensity = _intensity;
        data.color = _color;
        return data;
    }

    /// <summary>
    /// ポイントライトの設定用関数
    /// </summary>
    /// <param name="_position">座標</param>
    /// <param name="_intensity">強さ</param>
    /// <param name="_color">色</param>
    /// <param name="_range">ライトが当たる半径</param>
    /// <returns></returns>
    static LightData SetPointLightData(const Vector3& _position, float _intensity, const Vector3& _color, float _range)
    {
        LightData data;
        data.lightType = static_cast<uint32_t>(LightType::Point);
        data.position = _position;
        data.intensity = _intensity;
        data.color = _color;
        data.range = _range;
        return data;
    };
    /// <summary>
    /// スポットライトの設定の関数
    /// </summary>
    /// <param name="_position">座標</param>
    /// <param name="_intensity">強さ</param>
    /// <param name="_color">色</param>
    /// <param name="_range">ライトが当たる半径</param>
    /// <param name="_innerAngleDeg">内側の角度(度数法)</param>
    /// <param name="_outerAngleDeg">外側の角度(度数法)</param>
    /// <returns></returns>
    static LightData SetSpotLightData(const Vector3& _position, const Vector3& _direction, float _intensity,const Vector3& _color, float _range, float _innerAngleDeg, float _outerAngleDeg)
    {
        LightData data;
        data.lightType = static_cast<uint32_t>(LightType::Spot);
        data.position = _position;
        data.direction = _direction;
        data.direction.Normalize();
        data.intensity = _intensity;
        data.color = _color;
        data.range = _range;
        data.SetSpotAngle(_innerAngleDeg, _outerAngleDeg);
        return data;
    }

    void SetSpotAngle(float _innerAngleDeg, float _outerAngleDeg)
    {
        // inner > outer だと smoothstep が壊れるので入れ替える
        if (_innerAngleDeg > _outerAngleDeg)
        {
            std::swap(_innerAngleDeg, _outerAngleDeg);
        }

        innerCos = std::cos(DirectX::XMConvertToRadians(_innerAngleDeg));
        outerCos = std::cos(DirectX::XMConvertToRadians(_outerAngleDeg));

        // inner == outer だと smoothstep の幅が0になるので、ほんの少し幅を持たせる
        if (outerCos > innerCos - 0.0001f)
        {
            outerCos = innerCos - 0.0001f;
        }
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

    Vector3 cameraPos;
    float pad;

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

    // ポイント/スポットライト用
    void SetPosition(uint32_t _lightIndex, const Vector3& position);
    void SetRange(uint32_t _lightIndex, float range);
    void SetSpotAngle(uint32_t _lightIndex, float innerAngleDeg, float outerAngleDeg);

private:
    LightBufferData lightBufferData{};

    friend class SingletonBase<Light>;
};
