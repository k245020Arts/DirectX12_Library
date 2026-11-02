#pragma once

#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "../ResultErrorHandle/ResultErrorHandle.h"

struct Mesh;
struct Vertex;

struct aiMesh;
struct aiMaterial;
struct aiNode;
struct AnimationData;
struct aiScene;
struct ModelData;

struct ImportSettings // インポートするときのパラメータ
{
    const wchar_t* filename = nullptr; // ファイルパス

    bool inverseU = false; // U座標を反転させるか
    bool inverseV = false; // V座標を反転させるか
};

class AssimpLoader
{
public:
    ResultErrorHandle::ErrorCode  Load(const ImportSettings& setting, ModelData& _model); // モデルをロードする
    ResultErrorHandle::ErrorCode  LoadAnimation(const std::string& _filePath, ModelData& _model);
    ResultErrorHandle::ErrorCode  LoadAnimation(const std::wstring& _filePath, ModelData& _model);

    bool GetTextureLoader() { return textureLoader; }
private:
    void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);
    void LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* src);
    ResultErrorHandle::ErrorCode  LoadAnimations(const aiScene* scene,std::vector<AnimationData>& animations);
    ResultErrorHandle::ErrorCode  LoadBones(Mesh& dst, const aiMesh* src, ModelData& model);
    int LoadNodeHierarchy(const aiNode* src, int parentIndex, ModelData& model);
    void ApplyStaticMeshNodeTransforms(ModelData& model);

    bool textureLoader = false;
};
