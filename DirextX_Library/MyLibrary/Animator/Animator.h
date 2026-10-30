#pragma once
#include "../FBXModel/FBXModel.h"

//1ノード分のポーズ。行列ではなく S・R・T の成分で持つ
//行列のまま混ぜると回転が崩れるため、ブレンドは成分ごとに行う
struct NodePose
{
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
    Vector4 rotation = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    Vector3 translation = Vector3(1.0f, 1.0f, 1.0f);
};

class Animator
{
public:

    // 既定のブレンド時間（秒）
    static constexpr float DefaultBlendSeconds = 0.2f;
    
    Animator() = delete;
    Animator(const ModelData& _data);
    ~Animator();

    void Update(float deltaTime);

    const std::vector<Matrix>& GetBoneMatrices() const
    {
        return boneMatrices;
    }

    // 再生開始（animIndex は modelData.animations の番号）
    void Play(int animIndex, bool loop = true, float blendSeconds = DefaultBlendSeconds);
    void Stop() { playing = false; }
    void SetSpeed(float s) { speed = s; }

    void AddAnimation(const ModelData& _data);

    int GetAnimationIndex() { return currentAnimation; }
    
    bool GetPlaying() { return playing; }

    // ブレンド中かどうか
    bool IsBlending() const { return blending; }

private:
    // ボーンの姿勢を更新する処理
    void UpdateBonePose();

    // 学習用：特定のボーンを手動で曲げる
    void UpdateManualPose();

    // 本実装：FBXのモーションを再生する
    void UpdateAnimation(float deltaTime);

    // ボーンのグローバル行列を計算
    void CalculateGlobalTransforms();

    // スキニング用の行列を計算
    void CalculateBoneMatrices();

    // ---- ブレンド関連 ----
    // 元の姿勢originalLocalTransformsを S・R・T に分解して bindPoses を作る
    void BuildBindPoses();

    // チャンネル名、ノード番号の対応表を作る
    void BuildChannelBindings();

    // 再生時刻を進める
    void AdvanceTime(const AnimationData& anim, float deltaTime);

    // 指定時刻の姿勢を out に書き出す（キーが無いノードは元の姿勢のまま）
    void SamplePose(const AnimationData& anim, const std::vector<int>& nodeIds, double tick, std::vector<NodePose>& out) const;

    // 姿勢を行列に組み立ててノードのローカル変換へ書き込む
    void ApplyPoseToNodes(const std::vector<NodePose>& pose);


private:

    int    currentAnimation = -1;
    double currentTick = 0.0;
    float  speed = 1.0f;
    bool   loop = true;
    bool   playing = false;

    // [アニメ番号][チャンネル番号]->ノード番号
    std::vector<std::vector<int>> channelNodeIndices;

    ModelData modelData;

    std::vector<Matrix> boneMatrices;

    // 各ノードのグローバル変換行列
    std::vector<Matrix> globalTransforms;

    std::vector<DirectX::XMFLOAT4X4> originalLocalTransforms;

    void CalculateGlobalTransformsRecursive(int nodeIndex,const Matrix& parentTransform);

    //ブレンド用の状態 
    std::vector<NodePose> bindPoses;     // 元の姿勢
    std::vector<NodePose> poseCurrent;   // 今再生している動きの姿勢
    std::vector<NodePose> poseFrom;      // ブレンド元
    std::vector<NodePose> poseOut;       // 最終的にノードへ書き込む姿勢

    // いずれかのアニメーションで動かされたことのあるノード
    // 動かされていないノードは、分解せず元の行列をそのまま使う
    std::vector<uint8_t> driven;

    bool  hasPose = false;               // poseOut が有効（一度でもアニメーションを更新した）
    bool  blending = false;
    float blendTime = 0.0f;
    float blendElapsed = 0.0f;
};

