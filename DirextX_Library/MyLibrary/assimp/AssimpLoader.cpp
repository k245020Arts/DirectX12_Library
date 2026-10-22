#include "AssimpLoader.h"
#include "../ShaderStruct/ShaderStruct.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include "../FBXModel/FBXModel.h"

namespace fs = std::filesystem;

std::wstring GetDirectoryPath(const std::wstring& origin)
{
    fs::path p = origin.c_str();
    return p.remove_filename().c_str();
}

std::string ToUTF8(const std::wstring& value)
{
    auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
    auto buffer = new char[length];

    WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

    std::string result(buffer);
    delete[] buffer;
    buffer = nullptr;

    return result;
}

static DirectX::XMFLOAT4X4 ConvertMatrix(const aiMatrix4x4& a)
{
    // Assimpの変換行列をDirectXの行ベクトル規約に変換
    return DirectX::XMFLOAT4X4(a.a1, a.b1, a.c1, a.d1, a.a2, a.b2, a.c2, a.d2, a.a3, a.b3, a.c3, a.d3, a.a4, a.b4, a.c4, a.d4);
}

bool AssimpLoader::Load(const ImportSettings& setting,ModelData& _model)
{
    if (setting.filename == nullptr) {
        assert(false && "ファイルネームがないです");
        return false;
    }
    auto& meshes = setting.meshes;
    auto& animations = setting.animations;

    auto inverseU = setting.inverseU;
    auto inverseV = setting.inverseV;

    auto path = ToUTF8(setting.filename);

    Assimp::Importer importer;

    int flag = 0;

    flag |= aiProcess_Triangulate;
    //flag |= aiProcess_PreTransformVertices; //モーション付きのアニメーションを読み込むときに入らない
    flag |= aiProcess_CalcTangentSpace;
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_GenUVCoords;
    flag |= aiProcess_RemoveRedundantMaterials;
    flag |= aiProcess_OptimizeMeshes;

    auto scene = importer.ReadFile(path, flag);

    if (scene == nullptr) 
    {
        assert(false && "ファイルの読み込みエラー");
        return false;
    }

    meshes.clear();

    meshes.resize(scene->mNumMeshes);
    for (size_t i = 0; i < meshes.size(); i++) {
        const auto pMesh = scene->mMeshes[i];
        LoadMesh(meshes[i], pMesh, inverseU, inverseV);
        LoadBones(meshes[i], pMesh, _model);

        const auto materialIndex = pMesh->mMaterialIndex;
        if (materialIndex < scene->mNumMaterials)
        {
            const auto pMaterial = scene->mMaterials[materialIndex];
            LoadTexture(setting.filename, meshes[i], pMaterial);
        }
    }

    //アニメーション読み込み
    animations.clear();

    _model.rootNodeIndex =  LoadNodeHierarchy(scene->mRootNode, -1, _model);

    if (scene->mNumAnimations > 0)
    {
        LoadAnimations(scene, animations);
    }
    
    scene = nullptr;
    return true;
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV)
{
    aiVector3D zero3D(0.0f, 0.0f, 0.0f);
    aiColor4D zeroColor(1.0f, 1.0f, 1.0f, 1.0f);

    dst.Vertices.resize(src->mNumVertices);
    
    for (auto i = 0u; i < src->mNumVertices; ++i)
    {
        auto position = &(src->mVertices[i]);
        auto normal = src->HasNormals()? &src->mNormals[i] : &zero3D;
        auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
        auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;
        auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;

        //反転オプションが合ったらUVを反転させる
        if (inverseU) {
            uv->x = 1 - uv->x;
        }

        if (inverseV) {
            uv->y = 1 - uv->y;
        }

        VertexSkinning3D vertex = {};
        vertex.position = Vector3(position->x, position->y, position->z);
        vertex.normal = Vector3(normal->x, normal->y, normal->z);
        vertex.uv = Vector2(uv->x, uv->y);
        vertex.tangent = Vector3(tangent->x, tangent->y, tangent->z);
        vertex.color = Vector4(color->r, color->g, color->b,color->a);

        dst.Vertices[i] = vertex;
    }


    dst.Indices.resize(src->mNumFaces * 3);

    for (auto i = 0u; i < src->mNumFaces; ++i)
    {
        const auto& face = src->mFaces[i];

        dst.Indices[i * 3 + 0] = face.mIndices[0];
        dst.Indices[i * 3 + 1] = face.mIndices[1];
        dst.Indices[i * 3 + 2] = face.mIndices[2];
    }
}

void AssimpLoader::LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* src)
{
    aiString path;
    if (src->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
    {
        auto dir = GetDirectoryPath(filename);
        auto file = std::string(path.C_Str());
        dst.DiffuseMap = dir + GetWideString(file);
    }
    else {
        dst.DiffuseMap.clear();
    }
}

bool AssimpLoader::LoadAnimations(const aiScene* scene, std::vector<AnimationData>& animations)
{
    if (scene == nullptr)
    {
        return false;
    }

    animations.clear();

    if (scene->mNumAnimations == 0)
    {
        return false;
    }

    if (scene->mNumAnimations == 0)
    {
        OutputDebugStringA("FBXファイルにアニメーションがありません。\n");
        return false;
    }

    animations.reserve(scene->mNumAnimations);

    // ファイル内の全アニメーションを読み込む
    for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* srcAnimation =scene->mAnimations[i];

        AnimationData dstAnimation{};

        // アニメーション名
        dstAnimation.name = GetWideString(srcAnimation->mName.C_Str());

        // 名前が空の場合は仮の名前を設定
        if (dstAnimation.name.empty())
        {
            dstAnimation.name =L"Animation_" + std::to_wstring(i);
        }

        // 再生時間（tick単位）
        dstAnimation.duration = srcAnimation->mDuration;

        // 1秒あたりのtick数
        dstAnimation.ticksPerSecond = srcAnimation->mTicksPerSecond;

        dstAnimation.channels.reserve(srcAnimation->mNumChannels);

        // 各ノードのアニメーションを読み込む
        for (uint32_t j = 0;j < srcAnimation->mNumChannels;++j)
        {
            const aiNodeAnim* srcChannel = srcAnimation->mChannels[j];

            BoneChannel dstChannel{};

            // アニメーション対象のノード名
            dstChannel.nodeName = GetWideString(srcChannel->mNodeName.C_Str());

            //位置キーフレーム
            dstChannel.positionKeys.reserve(srcChannel->mNumPositionKeys);

            for (uint32_t k = 0;k < srcChannel->mNumPositionKeys;++k)
            {
                const aiVectorKey& key =srcChannel->mPositionKeys[k];

                Vector3 position(key.mValue.x, key.mValue.y,key.mValue.z);

                dstChannel.positionKeys.emplace_back(key.mTime,position);
            }

            // 回転キーフレーム
            // クォータニオン
            dstChannel.rotationKeys.reserve(srcChannel->mNumRotationKeys);

            for (uint32_t k = 0; k < srcChannel->mNumRotationKeys; ++k)
            {
                const aiQuatKey& key =srcChannel->mRotationKeys[k];

                // Vector4の格納順序は x, y, z, w
                Vector4 rotation(key.mValue.x,key.mValue.y,key.mValue.z,key.mValue.w );

                dstChannel.rotationKeys.emplace_back(key.mTime, rotation );
            }

            // --------------------------------
            // スケールキーフレーム
            // --------------------------------
            dstChannel.scalingKeys.reserve(srcChannel->mNumScalingKeys);

            for (uint32_t k = 0;k < srcChannel->mNumScalingKeys;++k)
            {
                const aiVectorKey& key =srcChannel->mScalingKeys[k];

                Vector3 scale(key.mValue.x,key.mValue.y,key.mValue.z);

                dstChannel.scalingKeys.emplace_back(key.mTime,scale);
            }

            dstAnimation.channels.emplace_back(std::move(dstChannel));
        }

        animations.emplace_back(std::move(dstAnimation));
    }

    return true;
}

bool AssimpLoader::LoadBones(Mesh& dst,const aiMesh* src,ModelData& model)
{
    for (uint32_t i = 0; i < src->mNumBones; ++i)
    {
        const aiBone* srcBone = src->mBones[i];
        const std::wstring name = GetWideString(srcBone->mName.C_Str());

        uint32_t boneId = 0;

        auto it = model.boneMap.find(name);

        if (it == model.boneMap.end())
        {
            boneId = static_cast<uint32_t>(model.bones.size());

            model.boneMap.emplace(name, boneId);

            BoneInfo info{};
            info.id = static_cast<int>(boneId);
            info.offsetMatrix = ConvertMatrix(srcBone->mOffsetMatrix);

            model.bones.push_back(info);
        }
        else
        {
            boneId = it->second;
        }

        // このメッシュの頂点にウェイトを登録
        for (uint32_t j = 0; j < srcBone->mNumWeights; ++j)
        {
            const aiVertexWeight& weight = srcBone->mWeights[j];

            if (weight.mVertexId >= dst.Vertices.size())
            {
                continue;
            }

            dst.Vertices[weight.mVertexId].AddBoneData(boneId,weight.mWeight);
        }
    }

    // ウェイトを正規化
    for (auto& vertex : dst.Vertices)
    {
        float total = 0.0f;

        for (float weight : vertex.weights)
        {
            total += weight;
        }

        if (total > 0.0f)
        {
            for (float& weight : vertex.weights)
            {
                weight /= total;
            }
        }
    }

    return true;
}

int AssimpLoader::LoadNodeHierarchy(const aiNode* src,int parentIndex,ModelData& model)
{
    // 現在のノードを登録する
    const int currentIndex = static_cast<int>(model.nodes.size());

    NodeData node{};
    node.name = GetWideString(src->mName.C_Str());
    node.parentIndex = parentIndex;

    // ローカル変換行列もここで保存する
    node.localTransform = ConvertMatrix(src->mTransformation);

    model.nodes.emplace_back(std::move(node));

    // 重要：子ノードを1つに限定せず、すべて処理する
    for (unsigned int i = 0; i < src->mNumChildren; ++i)
    {
        const int childIndex = LoadNodeHierarchy(src->mChildren[i],currentIndex,model);

        model.nodes[currentIndex].children.emplace_back(childIndex);
    }

    return currentIndex;
}