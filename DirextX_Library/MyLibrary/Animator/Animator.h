#pragma once
#include "../FBXModel/FBXModel.h"

class Animator
{
public:
    
    Animator() = delete;
    Animator(const ModelData& _data);
    ~Animator();

    void Update(float deltaTime);

    const std::vector<Matrix>& GetBoneMatrices() const
    {
        return boneMatrices;
    }

    // 再生開始（animIndex は modelData.animations の番号）
    void Play(int animIndex, bool loop = true);
    void Stop() { playing = false; }
    void SetSpeed(float s) { speed = s; }

    void AddAnimation(const ModelData& _data);

    int GetAnimationIndex() { return currentAnimation; }
    
    bool GetPlaying() { return playing; }

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
};

