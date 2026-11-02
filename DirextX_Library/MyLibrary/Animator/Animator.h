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

// ---------------------------------------------------------------------------
// IK（インバースキネマティクス）
// ---------------------------------------------------------------------------

// 関節の可動範囲（IKで曲げるときだけ使う。FKのアニメーションには影響しない）
struct IKJointLimit
{
    bool  useHinge = false;                 // true: 蝶番（1軸だけ曲がる。肘・膝など）
    DirectX::XMFLOAT3 axis{ 1, 0, 0 };      // 曲がる軸（そのボーンのローカル空間）
    float minAngle = 0.0f;                  // バインド姿勢からの角度の下限（ラジアン）
    float maxAngle = 0.0f;                  // 同・上限
};

// IKの1チェーン分（MMDのIKボーン1つ分に相当）
struct IKChain
{
    std::wstring name;
    IKSolverType type = IKSolverType::TwoBone;

    std::vector<int>          nodes;        // 付け根 → 先端 の順のノード番号（先端 = エフェクタ）
    std::vector<IKJointLimit> limits;       // nodes と同数（先端の分は使わない）

    // ---- 目標 ----
    DirectX::XMFLOAT3 target{};             // 目標位置（モデルのローカル空間）
    int  targetNode = -1;                   // 0以上なら、そのノードの位置を目標にする（MMDのIKボーン方式）

    // ---- ポール（TwoBone のみ）: 肘・膝が出る側の目安 ----
    DirectX::XMFLOAT3 pole{};
    int  poleNode = -1;
    bool usePole = false;

    // ---- CCD ----
    int   iterations = 10;                  // 繰り返し回数
    float limitAngle = 0.0f;                // 1回に回す最大角（ラジアン）。0 = 無制限
    float tolerance = 0.001f;              // 先端が目標に届いたとみなす距離（チェーン全長に対する割合）

    float weight = 0.0f;                   // 0 = FKのまま、1 = 完全にIK
    bool  enabled = true;
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

    // ---- IK ----
    // nodeNames: 付け根 → 先端 の順のノード名。TwoBone は3つ、CCD は2つ以上
    // 戻り値は IK の ID。失敗したら -1
    int  AddIK(const std::wstring& name, IKSolverType type, const std::vector<std::wstring>& nodeNames);
    int  FindIK(const std::wstring& name) const;

    // 目標をモデルのローカル空間で指定する（ワールド座標で指定するときは FBXModel 側を使う）
    void SetIKTarget(int ikId, const Vector3& modelLocalPos, float weight);
    // 別のノードの位置を目標にする（MMDのIKボーン方式）。nodeName が見つからなければ何もしない
    void SetIKTargetNode(int ikId, const std::wstring& nodeName, float weight);
    void SetIKPole(int ikId, const Vector3& modelLocalPos);
    void SetIKPoleNode(int ikId, const std::wstring& nodeName);
    void ClearIKPole(int ikId);
    // linkIndex 番目の関節を蝶番にする（角度はバインド姿勢からの値。ラジアン）
    void SetIKHinge(int ikId, int linkIndex, const Vector3& boneLocalAxis, float minAngle, float maxAngle);
    void SetIKWeight(int ikId, float weight);
    void SetIKEnabled(int ikId, bool enabled);

    // 蝶番の現在の角度（ラジアン）。可動範囲を決めるときの確認用
    float GetIKHingeAngle(int ikId, int linkIndex) const;

    // 細かい設定用（iterations などを直接書き換える）。AddIK を呼ぶと無効になるので、保持しないこと
    IKChain* GetIK(int ikId);

    // ---- ノード情報の取得 ----
    int  GetNodeCount() const { return static_cast<int>(modelData.nodes.size()); }
    const std::wstring& GetNodeName(int nodeIndex) const;

    // ノード名 → ノード番号（見つからなければ -1）
    // 毎フレーム使うなら、番号を一度だけ取得して保持しておくと、名前検索が不要になる
    int  GetNodeIndex(const std::wstring& nodeName) const;

    // モデルのローカル空間でのノードの姿勢（直近の Update の結果。IKの補正を含む）
    Matrix  GetNodeModelMatrix(int nodeIndex) const;
    Matrix  GetNodeModelMatrix(const std::wstring& nodeName) const;
    Vector3 GetNodeModelPosition(int nodeIndex) const;
    Vector3 GetNodeModelPosition(const std::wstring& nodeName) const;

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


    // ---- IK 関連 ----
    int  FindNode(const std::wstring& name) const;
    bool IsDescendant(int node, int ancestor) const;
    float HingeAngleOf(int node, const IKJointLimit& limit) const;
    void RotateNodeAroundPosition(int node, DirectX::FXMVECTOR q);
    void ApplyIK();
    void SolveTwoBoneIK(const IKChain& ik, DirectX::FXMVECTOR target, DirectX::FXMVECTOR pole, bool hasPole);
    void SolveCCD(const IKChain& ik, DirectX::FXMVECTOR target);

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

    // IK
    std::vector<IKChain> ikChains;

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

