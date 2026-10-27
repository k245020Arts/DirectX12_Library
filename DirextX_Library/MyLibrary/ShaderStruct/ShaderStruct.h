#pragma once
#include <Windows.h>
#include <vector>
#include <DirectXMath.h>
#include "../../DirectX12_Library/d3dx12.h"
#include "../Helper/Helper.h"
#include "../Extension/Extension.h"
#include "../Easing/Easing.h"

struct Vertex
{
    Vector3 position; // 位置座標
    Vector3 normal; // 法線
    Vector2 uv; // uv座標
    Vector3 tangent; // 接空間
    Vector4 color; // 頂点色

   static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
    static const int InputElementCount = 5;
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) MatrixTransform
{
    DirectX::XMMATRIX World; // ワールド行列
    DirectX::XMMATRIX View; // ビュー行列
    DirectX::XMMATRIX Proj; // 投影行列

    DirectX::XMFLOAT4 uvRect; //切り取り座標(本当は分けたほうが良いと思うが、容量的にはこっちの方が少なく管理できると考えたためとりあえずこっちに)
};

struct Transform
{
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;

    Transform() {
        position = Vector3();
        rotation = Vector3();
        scale = Vector3(1.0f,1.0f,1.0f);
    }

    void SetPosition(const Vector3& _position)
    {
        position = _position;
    };

    void SetRotate(const Vector3& _rotate)
    {
        rotation = _rotate;
    };

    void SetScale(const Vector3& _scale)
    {
        scale = _scale;
    };
};

//各ボーンのキーフレーム
struct BoneChannel {
    std::wstring nodeName;
    std::vector<std::pair<double, Vector3>> positionKeys;
    std::vector<std::pair<double, Vector4>> rotationKeys; // クォータニオン
    std::vector<std::pair<double, Vector3>> scalingKeys;
};

//モーションデータ（1ファイル内の1つのアニメーション）
struct AnimationData {
    std::wstring name;
    double duration = 0.0;
    double ticksPerSecond = 0.0;
    std::vector<BoneChannel> channels;
};

//ボーンのオフセット情報（メッシュ空間からボーン空間への変換行列）
struct BoneInfo {
    int id;
    std::wstring name;
    int nodeIndex = -1;
    DirectX::XMFLOAT4X4 offsetMatrix;
};

//
struct VertexSkinning3D {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Vector3 tangent;
    Vector4 color;

    // アニメーション用に以下を追加（最大4つのボーンが影響する場合）
    uint32_t boneno[4] = { 0, 0, 0, 0 };
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // ウェイトを安全に追加するためのヘルパー
    void AddBoneData(uint32_t boneId, float _weight) {
        for (int i = 0; i < 4; ++i) {
            if (weights[i] == 0.0f) {
                boneno[i] = boneId;
                weights[i] = _weight;
                return;
            }
        }
    }

    static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
    static const int InputElementCount = 7;
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct Mesh
{
    std::vector<VertexSkinning3D> Vertices; // 頂点データの配列
    std::vector<uint32_t> Indices; // インデックスの配列
    std::wstring DiffuseMap; // テクスチャのファイルパス

    bool hasBones = false;
};

enum class BlendState
{
    NO_BLEND,
    ALPHA,
    ADD,
    SUB,
    MUL,
};

