#include "Animator.h"

Animator::Animator(const ModelData& _data)
{
    modelData = _data;
    boneMatrices.resize(_data.boneMap.size());
}

Animator::~Animator()
{
}

void Animator::Update(float deltaTime)
{
    // 学習段階：手動でボーンを曲げる
    UpdateManualPose();

    CalculateGlobalTransforms();
    CalculateBoneMatrices();

    // モーション実装後は、こちらに切り替える
    // UpdateAnimation(deltaTime);
}

void Animator::UpdateBonePose()
{
}

/*[L"a_LeftMiddle 12"]
    [L"d_Spine1"]*/

void Animator::UpdateManualPose()
{
    auto index = modelData.boneMap[L"d_RightForeArm"];
    
    boneMatrices[index] += DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2);
}

void Animator::UpdateAnimation(float deltaTime)
{
}

void Animator::CalculateGlobalTransforms()
{
}

void Animator::CalculateBoneMatrices()
{
}
