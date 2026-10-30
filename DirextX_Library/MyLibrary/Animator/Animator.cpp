#include "Animator.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace
{
    //tの値の直前のキー番号を返す
    // (0,2,4,6,8,10)とならんでて、tが3だったら2が入っているキーの番号、つまり1を返す
    // (キーのサイズが2以上前提)
    template<typename T>
    size_t FindKey(const std::vector<std::pair<double, T>>& keys, double t)
    {
        auto it = std::upper_bound(keys.begin(), keys.end(), t,
            [](double v, const std::pair<double, T>& k) { return v < k.first; });

        if (it == keys.begin()) 
            return 0;

        size_t i = static_cast<size_t>(it - keys.begin()) - 1;
        return std::min(i, keys.size() - 2);
    }

    /// <summary>
    /// 2つのキーの間で、現在時刻がどこまで進んだかを 0.0〜1.0 で返す
    /// (0.0 = 左キーの位置、1.0 = 右キーの位置)
    /// </summary>
    /// <param name="t0">左側のキーの時刻</param>
    /// <param name="t1">右側のキーの時刻</param>
    /// <param name="t">現在の再生時刻</param>
    /// <returns>補間係数。範囲外は 0〜1 に丸める</returns>
    float Factor(double t0, double t1, double t)
    {
        const double d = t1 - t0;
        if (d <= 0.0) 
            return 0.0f;
        return std::clamp(static_cast<float>((t - t0) / d), 0.0f, 1.0f);
    }

    // 位置・スケール用（線形補間）。キーが無ければ false
    bool SampleVector3(const std::vector<std::pair<double, Vector3>>& keys,double t, DirectX::XMVECTOR& out)
    {
        if (keys.empty()) 
            return false;
        if (keys.size() == 1)
        {
            out = DirectX::XMLoadFloat3(&keys[0].second);
            return true;
        }
        const size_t i = FindKey(keys, t);
        const float  f = Factor(keys[i].first, keys[i + 1].first, t);
        out = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&keys[i].second),DirectX::XMLoadFloat3(&keys[i + 1].second), f);
        return true;
    }

    // 回転用（球面線形補間）
    bool SampleQuotation(const std::vector<std::pair<double, Vector4>>& keys,double t, DirectX::XMVECTOR& out)
    {
        if (keys.empty())
            return false;
        if (keys.size() == 1)
        {
            out = DirectX::XMLoadFloat4(&keys[0].second);
            return true;
        }
        const size_t i = FindKey(keys, t);
        float  f = Factor(keys[i].first, keys[i + 1].first, t);
        f = Easing::SmoothStep<float>(0.0f, 1.0f, f);
        out = DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&keys[i].second),DirectX::XMLoadFloat4(&keys[i + 1].second), f);
        out = DirectX::XMQuaternionNormalize(out);
        return true;
    }

    // 2つのポーズを f(0〜1) で混ぜる
    // 位置・スケールは線形補間、回転は球面線形補間（最短経路）
    NodePose BlendPose(const NodePose& a, const NodePose& b, float f)
    {
        NodePose o;

        DirectX::XMStoreFloat3(&o.scale,DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&a.scale), DirectX::XMLoadFloat3(&b.scale), f));

        DirectX::XMStoreFloat3(&o.translation,DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&a.translation), DirectX::XMLoadFloat3(&b.translation), f));

        DirectX::XMStoreFloat4(&o.rotation,DirectX::XMQuaternionNormalize(DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&a.rotation), DirectX::XMLoadFloat4(&b.rotation), f)));

        return o;
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

    AddAnimation(modelData);

    BuildBindPoses();
    driven.assign(modelData.nodes.size(), 0);
    BuildChannelBindings();
}

Animator::~Animator()
{
}

void Animator::Update(float deltaTime)
{
    //ノードのローカル変換を更新
    //UpdateManualPose();

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

    int nameIndex = 98;

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

    //ここで、ボーンの位置を原点に戻し、回転させている。
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

        //先ほど計算した回転処理を取得
        Matrix offset = DirectX::XMLoadFloat4x4(&b.offsetMatrix);
        //ここでボーンの位置を、原点に戻している。
        boneMatrices[i] = offset * globalTransforms[b.nodeIndex] * modelData.globalInverseTransform;;
    }
}

void Animator::Play(int animIndex, bool _loop, float blendSeconds)
{
    if (animIndex < 0 || animIndex >= static_cast<int>(modelData.animations.size()))
        return;

    //ループ再生中の同じ動きがもう一度要求されても、最初に戻さない
    //毎フレーム Play を呼ぶ使い方でも、動きが止まらないようにする
    if (animIndex == currentAnimation && playing && loop && _loop)
        return;

    //直前のポーズを保存して、新しい動きへのブレンドを始める
    //ブレンド中に再度切り替えた場合も、poseOut（混ぜ途中のポーズ）から滑らかにつながる
    if (blendSeconds > 0.0f && hasPose)
    {
        poseFrom = poseOut;
        blending = true;
        blendTime = blendSeconds;
        blendElapsed = 0.0f;
    }
    else
    {
        blending = false;
    }

    //この動きが動かすノードを記録する（動かされたノードだけ、成分から行列を組み立てる）
    for (int nodeIndex : channelNodeIndices[animIndex])
    {
        if (nodeIndex >= 0 && nodeIndex < static_cast<int>(driven.size()))
            driven[nodeIndex] = 1;
    }

    currentAnimation = animIndex;
    currentTick = 0.0;
    loop = _loop;
    playing = true;
}

void Animator::AddAnimation(const ModelData& _data)
{
    modelData = _data;

    // ノード数が変わっていたら（別のモデルが読み込まれた場合など）、ノードに依存するバッファを作り直す
    if (originalLocalTransforms.size() != modelData.nodes.size())
    {
        originalLocalTransforms.clear();
        for (const auto& node : modelData.nodes)
        {
            originalLocalTransforms.push_back(node.localTransform);
        }

        globalTransforms.resize(modelData.nodes.size());
        BuildBindPoses();
        driven.assign(modelData.nodes.size(), 0);

        currentAnimation = -1;
        playing = false;
        blending = false;
        hasPose = false;
    }

    boneMatrices.resize(modelData.bones.size());

    BuildChannelBindings();
}

void Animator::BuildChannelBindings()
{
    std::unordered_map<std::wstring, int> nodeByName;
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
    {
        nodeByName.emplace(modelData.nodes[i].name, static_cast<int>(i));
    }

    channelNodeIndices.clear();

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

void Animator::BuildBindPoses()
{
    bindPoses.resize(originalLocalTransforms.size());

    for (size_t i = 0; i < bindPoses.size(); ++i)
    {
        Matrix m = DirectX::XMLoadFloat4x4(&originalLocalTransforms[i]);

        DirectX::XMVECTOR s, r, t;
        if (!DirectX::XMMatrixDecompose(&s, &r, &t, m))
        {
            // 分解できない行列（スケール0など）は、位置だけ取り出して他は初期値にする
            s = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
            r = DirectX::XMQuaternionIdentity();
            t = m.r[3];
        }

        DirectX::XMStoreFloat3(&bindPoses[i].scale, s);
        DirectX::XMStoreFloat4(&bindPoses[i].rotation, r);
        DirectX::XMStoreFloat3(&bindPoses[i].translation, t);
    }

    poseCurrent = bindPoses;
    poseFrom = bindPoses;
    poseOut = bindPoses;
}

void Animator::AdvanceTime(const AnimationData& anim, float deltaTime)
{
    if (!playing)
        return;

    // ticksPerSecond が 0 のときは Assimp の既定値 25 を使う
    const double tps = (anim.ticksPerSecond > 0.0) ? anim.ticksPerSecond : 25.0;

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

void Animator::SamplePose(const AnimationData& anim, const std::vector<int>& nodeIds, double tick, std::vector<NodePose>& out) const
{
    // 全ノードを元の姿勢に戻してから、チャンネルのあるノードだけ上書きする
    out = bindPoses;

    for (size_t c = 0; c < anim.channels.size(); ++c)
    {
        const int nodeIndex = nodeIds[c];
        if (nodeIndex < 0)
            continue;

        const BoneChannel& ch = anim.channels[c];

        // キーが無い成分は、元の姿勢の値をそのまま使う
        DirectX::XMVECTOR s = DirectX::XMLoadFloat3(&out[nodeIndex].scale);
        DirectX::XMVECTOR r = DirectX::XMLoadFloat4(&out[nodeIndex].rotation);
        DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&out[nodeIndex].translation);

        SampleVector3(ch.scalingKeys, tick, s);
        SampleQuotation(ch.rotationKeys, tick, r);
        SampleVector3(ch.positionKeys, tick, t);

        DirectX::XMStoreFloat3(&out[nodeIndex].scale, s);
        DirectX::XMStoreFloat4(&out[nodeIndex].rotation, r);
        DirectX::XMStoreFloat3(&out[nodeIndex].translation, t);
    }
}

void Animator::ApplyPoseToNodes(const std::vector<NodePose>& pose)
{
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
    {
        // 一度も動かされていないノードは、元の行列をそのまま使う
        // 分解と再構成による誤差や、せん断の欠落を避けるため
        if (!driven[i])
        {
            modelData.nodes[i].localTransform = originalLocalTransforms[i];
            continue;
        }

        // SRTの順番で掛ける
        Matrix local =
            DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&pose[i].scale)) *
            DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&pose[i].rotation)) *
            DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pose[i].translation));

        DirectX::XMStoreFloat4x4(&modelData.nodes[i].localTransform, local);
    }
}

void Animator::UpdateAnimation(float deltaTime)
{
    if (currentAnimation < 0)  //アニメーションが設定されていなければやらない
        return;

    const AnimationData& anim = modelData.animations[currentAnimation];

    //時間を進める
    AdvanceTime(anim, deltaTime);

    //今の動きの姿勢を取り出す
    SamplePose(anim, channelNodeIndices[currentAnimation], currentTick, poseCurrent);

    //ブレンド中なら、切り替え前のポーズと混ぜる
    if (blending)
    {
        blendElapsed += deltaTime;

        float f = (blendTime > 0.0f) ? std::clamp(blendElapsed / blendTime, 0.0f, 1.0f) : 1.0f;

        if (f >= 1.0f)
            blending = false;

        // 混ぜ始めと終わりをなめらかにする（直線のままなら f をそのまま使う）
        const float w = Easing::SmoothStep<float>(0.0f, 1.0f, f);

        for (size_t i = 0; i < poseOut.size(); ++i)
        {
            poseOut[i] = BlendPose(poseFrom[i], poseCurrent[i], w);
        }
    }
    else
    {
        poseOut = poseCurrent;
    }

    hasPose = true;

    //最終ポーズをノードのローカル変換へ書き込む
    ApplyPoseToNodes(poseOut);
}

void Animator::CalculateGlobalTransforms()
{
    if (modelData.rootNodeIndex < 0 || modelData.rootNodeIndex >= static_cast<int>(modelData.nodes.size()))
    {
        return;
    }

    Matrix identity;
    identity = DirectX::XMMatrixIdentity();

    CalculateGlobalTransformsRecursive(modelData.rootNodeIndex, identity);
}

void Animator::CalculateGlobalTransformsRecursive(int nodeIndex, const Matrix& parentTransform)
{
    const NodeData& node = modelData.nodes[nodeIndex];

    Matrix localTransform;
    localTransform = DirectX::XMLoadFloat4x4(&node.localTransform);

    //ローカル変換の後に親の変換を適用
    globalTransforms[nodeIndex] = localTransform * parentTransform;

    for (uint32_t childIndex : node.children)
    {
        //子供のボーンにも回転を適用させないといけない
        CalculateGlobalTransformsRecursive(static_cast<int>(childIndex), globalTransforms[nodeIndex]);
    }
}