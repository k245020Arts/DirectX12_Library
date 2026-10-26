#include "Animator.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace
{
    // t を挟む区間の左側のキー番号を返す（keys.size() >= 2 が前提）
    template<typename T>
    size_t FindKey(const std::vector<std::pair<double, T>>& keys, double t)
    {
        auto it = std::upper_bound(keys.begin(), keys.end(), t,
            [](double v, const std::pair<double, T>& k) { return v < k.first; });

        if (it == keys.begin()) return 0;

        size_t i = static_cast<size_t>(it - keys.begin()) - 1;
        return std::min(i, keys.size() - 2);
    }

    float Factor(double t0, double t1, double t)
    {
        const double d = t1 - t0;
        if (d <= 0.0) return 0.0f;
        return std::clamp(static_cast<float>((t - t0) / d), 0.0f, 1.0f);
    }

    // 位置・スケール用（線形補間）。キーが無ければ false
    bool SampleVec3(const std::vector<std::pair<double, Vector3>>& keys,
        double t, DirectX::XMVECTOR& out)
    {
        if (keys.empty()) return false;
        if (keys.size() == 1)
        {
            out = DirectX::XMLoadFloat3(&keys[0].second);
            return true;
        }
        const size_t i = FindKey(keys, t);
        const float  f = Factor(keys[i].first, keys[i + 1].first, t);
        out = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&keys[i].second),
            DirectX::XMLoadFloat3(&keys[i + 1].second), f);
        return true;
    }

    // 回転用（球面線形補間）
    bool SampleQuat(const std::vector<std::pair<double, Vector4>>& keys,
        double t, DirectX::XMVECTOR& out)
    {
        if (keys.empty()) return false;
        if (keys.size() == 1)
        {
            out = DirectX::XMLoadFloat4(&keys[0].second);
            return true;
        }
        const size_t i = FindKey(keys, t);
        const float  f = Factor(keys[i].first, keys[i + 1].first, t);
        out = DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&keys[i].second),
            DirectX::XMLoadFloat4(&keys[i + 1].second), f);
        out = DirectX::XMQuaternionNormalize(out);
        return true;
    }
}

Animator::Animator(const ModelData& _data)
{
    modelData = _data;
    globalTransforms.resize(modelData.nodes.size());
    boneMatrices.resize(modelData.bones.size());

    for (const auto& node : modelData.nodes)
    {
        originalLocalTransforms.push_back(node.localTransform);
    }

    // チャンネル名 -> ノード番号を、ロード時に1回だけ解決する
    std::unordered_map<std::wstring, int> nodeByName;
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
        nodeByName.emplace(modelData.nodes[i].name, static_cast<int>(i));

    channelNodeIndices.resize(modelData.animations.size());
    for (size_t a = 0; a < modelData.animations.size(); ++a)
    {
        const auto& anim = modelData.animations[a];
        channelNodeIndices[a].assign(anim.channels.size(), -1);

        for (size_t c = 0; c < anim.channels.size(); ++c)
        {
            auto it = nodeByName.find(anim.channels[c].nodeName);
            if (it != nodeByName.end())
                channelNodeIndices[a][c] = it->second;
            else
                OutputDebugStringW((L"チャンネルに対応するノードがありません: " + anim.channels[c].nodeName + L"\n").c_str());
        }
    }
}

Animator::~Animator()
{
}

void Animator::Update(float deltaTime)
{
    //ノードのローカル変換を更新
   // UpdateManualPose();

    if (currentAnimation >= 0)
        UpdateAnimation(deltaTime);


    //ノード階層からグローバル変換を計算
    CalculateGlobalTransforms();
    // グローバル変換からボーン行列を計算
    CalculateBoneMatrices();

    // モーション実装後は、こちらに切り替える
    // UpdateAnimation(deltaTime);
}

void Animator::UpdateBonePose()
{
}

/*[L"a_LeftMiddle 12"]
    [L"d_Spine1"]*/



//L"mixamorig:LeftShoulder_$AssimpFbx$_Rotation"
//L"mixamorig:LeftShoulder"
//L"mixamorig:LeftArm_$AssimpFbx$_Translation"
//L"mixamorig:LeftArm_$AssimpFbx$_PreRotation"
//L"mixamorig:LeftArm_$AssimpFbx$_Rotation"
//L"mixamorig:LeftArm"
//L"mixamorig:LeftForeArm_$AssimpFbx$_Translatic"


float testRotation = 0.0f;

void Animator::UpdateManualPose()
{
    //auto index = modelData.boneMap[L"d_RightForeArm"];
    //auto index = modelData.boneMap[L"a_LeftMiddle"];
    //auto index = modelData.boneMap[L"d_Head"];
    
    //boneMatrices[index] = DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2);

    // Mixamo: L"mixamorig:RightForeArm" / 元モデル: L"d_RightForeArm"
    const std::wstring target = L"d_RightForeArm";

    int nameIndex = 126;

    int nodeIndex = -1;
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
    {
        if (modelData.nodes[i].name == modelData.nodes[nameIndex].name)
        {
            nodeIndex = static_cast<int>(i);
            break;
        }
    }
    if (nodeIndex < 0) 
        return;

    testRotation += 1.0f;
    Matrix original = DirectX::XMLoadFloat4x4(&originalLocalTransforms[nodeIndex]);
    Matrix rotation = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(testRotation));

    // 行ベクトル: 回転を左から掛けると、関節位置を中心に回る
    DirectX::XMStoreFloat4x4(&modelData.nodes[nodeIndex].localTransform, rotation * original);
}

void Animator::CalculateBoneMatrices()
{
    for (size_t i = 0; i < modelData.bones.size(); ++i)
    {
        const BoneInfo& b = modelData.bones[i];

        if (b.nodeIndex < 0)
        {
            boneMatrices[i] = Matrix::Identity();
            continue;
        }

        Matrix offset = DirectX::XMLoadFloat4x4(&b.offsetMatrix);
        boneMatrices[i] = offset * globalTransforms[b.nodeIndex];
    }
}

void Animator::Play(int animIndex, bool _loop)
{
    if (animIndex < 0 || animIndex >= static_cast<int>(modelData.animations.size()))
        return;

    currentAnimation = animIndex;
    currentTick = 0.0;
    loop = _loop;
    playing = true;
}

void Animator::UpdateAnimation(float deltaTime)
{
    if (currentAnimation < 0) return;

    const AnimationData& anim = modelData.animations[currentAnimation];

    // 1. 時間を進める（ticksPerSecond が 0 のときは Assimp の既定値 25 を使う）
    const double tps = (anim.ticksPerSecond > 0.0) ? anim.ticksPerSecond : 25.0;

    if (playing)
    {
        currentTick += static_cast<double>(deltaTime) * speed * tps;

        if (anim.duration > 0.0)
        {
            if (loop)
            {
                currentTick = std::fmod(currentTick, anim.duration);
            }
            else if (currentTick >= anim.duration)
            {
                currentTick = anim.duration;
                playing = false;
            }
        }
    }

    // 2. 全ノードを元の姿勢に戻す
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
        modelData.nodes[i].localTransform = originalLocalTransforms[i];

    // 3 & 4. チャンネルごとに補間して、ローカル変換を上書き
    const auto& nodeIds = channelNodeIndices[currentAnimation];

    for (size_t c = 0; c < anim.channels.size(); ++c)
    {
        const int nodeIndex = nodeIds[c];
        if (nodeIndex < 0) continue;

        const BoneChannel& ch = anim.channels[c];

        // キーが無い成分は、元の姿勢から分解して使う
        DirectX::XMVECTOR s, r, t;
        DirectX::XMMATRIX orig = DirectX::XMLoadFloat4x4(&originalLocalTransforms[nodeIndex]);
        DirectX::XMMatrixDecompose(&s, &r, &t, orig);

        SampleVec3(ch.scalingKeys, currentTick, s);
        SampleQuat(ch.rotationKeys, currentTick, r);
        SampleVec3(ch.positionKeys, currentTick, t);

        // 行ベクトル規約: S → R → T の順
        DirectX::XMMATRIX local =
            DirectX::XMMatrixScalingFromVector(s) *
            DirectX::XMMatrixRotationQuaternion(r) *
            DirectX::XMMatrixTranslationFromVector(t);

        DirectX::XMStoreFloat4x4(&modelData.nodes[nodeIndex].localTransform, local);
    }
}

void Animator::CalculateGlobalTransforms()
{
    if (modelData.rootNodeIndex < 0 ||modelData.rootNodeIndex >= static_cast<int>(modelData.nodes.size()))
    {
        return;
    }

    Matrix identity;
    identity = DirectX::XMMatrixIdentity();

    CalculateGlobalTransformsRecursive(modelData.rootNodeIndex,identity);
}

void Animator::CalculateGlobalTransformsRecursive(int nodeIndex,const Matrix& parentTransform)
{
    const NodeData& node = modelData.nodes[nodeIndex];

    Matrix localTransform;
    localTransform = DirectX::XMLoadFloat4x4(& node.localTransform);

    // 行ベクトル規約：ローカル変換の後に親の変換を適用
    globalTransforms[nodeIndex] = localTransform * parentTransform;

    for (uint32_t childIndex : node.children)
    {
        CalculateGlobalTransformsRecursive(static_cast<int>(childIndex),globalTransforms[nodeIndex]);
    }
}
