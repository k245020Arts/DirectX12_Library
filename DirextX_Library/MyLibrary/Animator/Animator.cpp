#include "Animator.h"

Animator::Animator(const ModelData& _data)
{
    modelData = _data;
    globalTransforms.resize(modelData.nodes.size());
    boneMatrices.resize(modelData.bones.size());

    for (const auto& node : modelData.nodes)
    {
        originalLocalTransforms.push_back(node.localTransform);
    }

}

Animator::~Animator()
{
}

void Animator::Update(float deltaTime)
{
    // 1. ノードのローカル変換を更新
    UpdateManualPose();

    // 2. ノード階層からグローバル変換を計算
    CalculateGlobalTransforms();

    // 3. グローバル変換からボーン行列を計算
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
    const std::wstring target = L"a_LeftMiddle";

    int nodeIndex = -1;
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
    {
        if (modelData.nodes[i].name == target)
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

void Animator::UpdateAnimation(float deltaTime)
{
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
