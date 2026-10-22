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
    ModelData modelData;

    std::vector<Matrix> boneMatrices;
};

