#pragma once
#include <Windows.h>
#include <vector>
#include <DirectXMath.h>
#include "../../DirectX12_Library/d3dx12.h"

struct Vertex
{
    DirectX::XMFLOAT3 position; // 位置座標
    DirectX::XMFLOAT3 normal; // 法線
    DirectX::XMFLOAT2 uv; // uv座標
    DirectX::XMFLOAT3 tangent; // 接空間
    DirectX::XMFLOAT4 color; // 頂点色

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

typedef DirectX::XMFLOAT4 Vector4;
typedef DirectX::XMFLOAT3 Vector3;
typedef DirectX::XMFLOAT2 Vector2;

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

struct Mesh
{
    std::vector<Vertex> Vertices; // 頂点データの配列
    std::vector<uint32_t> Indices; // インデックスの配列
    std::wstring DiffuseMap; // テクスチャのファイルパス
};

enum class BlendState
{
    NO_BLEND,
    ALPHA,
    ADD,
    SUB,
    MUL,
};