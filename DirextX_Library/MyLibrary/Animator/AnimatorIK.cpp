#include "Animator.h"

#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Animator の IK 部分
//
// 全体の流れ（Animator::Update）
//   UpdateAnimation  : FK。ブレンド後のポーズをノードのローカル変換へ書き込む
//   CalculateGlobalTransforms
//   ApplyIK          : FKの結果に対して、先端を目標へ近づけるようにローカル変換を補正する
//   CalculateBoneMatrices
//
// IKの結果は poseOut には書き戻さない。毎フレーム FK からやり直すので、補正が累積しない。
// 座標は「ノード階層の空間」（globalTransforms の空間）で計算する。
// 外から受け取る目標位置は「モデルのローカル空間」なので、globalInverseTransform の逆行列で変換する。
// ---------------------------------------------------------------------------

namespace
{
    using namespace DirectX;

    // from の向きを to の向きへ回すクォータニオン
    XMVECTOR RotationFromTo(FXMVECTOR from, FXMVECTOR to)
    {
        const XMVECTOR a = XMVector3Normalize(from);
        const XMVECTOR b = XMVector3Normalize(to);
        const float d = XMVectorGetX(XMVector3Dot(a, b));

        if (d > 0.99999f)
            return XMQuaternionIdentity();                 // ほぼ同じ向き

        if (d < -0.99999f)                                 // 真逆: a に垂直な軸で180度
        {
            XMVECTOR axis = XMVector3Cross(a, XMVectorSet(1, 0, 0, 0));
            if (XMVectorGetX(XMVector3LengthSq(axis)) < 1e-6f)
                axis = XMVector3Cross(a, XMVectorSet(0, 1, 0, 0));
            return XMQuaternionRotationAxis(XMVector3Normalize(axis), XM_PI);
        }

        // (a×b, 1+a·b) を正規化すると、a→b の回転になる
        return XMQuaternionNormalize(XMVectorSetW(XMVector3Cross(a, b), 1.0f + d));
    }

    // 回転角が maxAngle を超えていたら、同じ軸のまま maxAngle に丸める（q は最短回転 = w >= 0）
    XMVECTOR ClampRotationAngle(FXMVECTOR q, float maxAngle)
    {
        if (maxAngle <= 0.0f)
            return q;

        XMVECTOR axis;
        float angle = 0.0f;
        XMQuaternionToAxisAngle(&axis, &angle, q);

        if (angle <= maxAngle)
            return q;

        return XMQuaternionRotationAxis(axis, maxAngle);
    }

    // ローカル行列から回転（クォータニオン）だけを取り出す
    XMVECTOR LocalRotationOf(const XMFLOAT4X4& m)
    {
        XMVECTOR s, r, t;
        if (!XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m)))
            return XMQuaternionIdentity();
        return r;
    }

    // v から、単位ベクトル axis 方向の成分を取り除く
    XMVECTOR RemoveComponent(FXMVECTOR v, FXMVECTOR axis)
    {
        return v - axis * XMVector3Dot(v, axis);
    }
}

// ---------------------------------------------------------------------------
// 登録・設定
// ---------------------------------------------------------------------------

int Animator::FindNode(const std::wstring& name) const
{
    for (size_t i = 0; i < modelData.nodes.size(); ++i)
    {
        if (modelData.nodes[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

// node が ancestor の子孫か（間に補助ノードが挟まっていてもよい）
bool Animator::IsDescendant(int node, int ancestor) const
{
    int p = modelData.nodes[node].parentIndex;
    while (p >= 0)
    {
        if (p == ancestor)
            return true;
        p = modelData.nodes[p].parentIndex;
    }
    return false;
}

int Animator::AddIK(const std::wstring& name, IKSolverType type, const std::vector<std::wstring>& nodeNames)
{
    if (type == IKSolverType::TwoBone && nodeNames.size() != 3)
    {
        OutputDebugStringW((L"IK(" + name + L"): TwoBone はノード名を3つ指定してください\n").c_str());
        return -1;
    }
    if (nodeNames.size() < 2)
    {
        OutputDebugStringW((L"IK(" + name + L"): ノード名は2つ以上必要です\n").c_str());
        return -1;
    }

    IKChain ik;
    ik.name = name;
    ik.type = type;

    for (const auto& n : nodeNames)
    {
        const int index = FindNode(n);
        if (index < 0)
        {
            OutputDebugStringW((L"IK(" + name + L"): ノードが見つかりません: " + n + L"\n").c_str());
            return -1;
        }
        ik.nodes.push_back(index);
    }

    // 付け根 → 先端 の順に、親子関係になっているか確認
    for (size_t i = 0; i + 1 < ik.nodes.size(); ++i)
    {
        if (!IsDescendant(ik.nodes[i + 1], ik.nodes[i]))
        {
            OutputDebugStringW((L"IK(" + name + L"): ノードの順序が、付け根 → 先端 の親子関係になっていません\n").c_str());
            return -1;
        }
    }

    ik.limits.resize(ik.nodes.size());

    ikChains.push_back(std::move(ik));
    return static_cast<int>(ikChains.size()) - 1;
}

int Animator::FindIK(const std::wstring& name) const
{
    for (size_t i = 0; i < ikChains.size(); ++i)
    {
        if (ikChains[i].name == name)
            return static_cast<int>(i);
    }
    return -1;
}

IKChain* Animator::GetIK(int ikId)
{
    if (ikId < 0 || ikId >= static_cast<int>(ikChains.size()))
        return nullptr;
    return &ikChains[ikId];
}

void Animator::SetIKTarget(int ikId, const Vector3& pos, float weight)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    ik->target = pos;
    ik->targetNode = -1;
    ik->weight = std::clamp(weight, 0.0f, 1.0f);
}

void Animator::SetIKTargetNode(int ikId, const std::wstring& nodeName, float weight)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    const int node = FindNode(nodeName);
    if (node < 0)
    {
        OutputDebugStringW((L"IK: 目標ノードが見つかりません: " + nodeName + L"\n").c_str());
        return;
    }

    ik->targetNode = node;
    ik->weight = std::clamp(weight, 0.0f, 1.0f);
}

void Animator::SetIKPole(int ikId, const Vector3& pos)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    ik->pole = pos;
    ik->poleNode = -1;
    ik->usePole = true;
}

void Animator::SetIKPoleNode(int ikId, const std::wstring& nodeName)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    const int node = FindNode(nodeName);
    if (node < 0)
    {
        OutputDebugStringW((L"IK: ポールのノードが見つかりません: " + nodeName + L"\n").c_str());
        return;
    }

    ik->poleNode = node;
    ik->usePole = true;
}

void Animator::ClearIKPole(int ikId)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    ik->usePole = false;
    ik->poleNode = -1;
}

void Animator::SetIKHinge(int ikId, int linkIndex, const Vector3& axis, float minAngle, float maxAngle)
{
    IKChain* ik = GetIK(ikId);
    if (!ik) return;

    // 先端（最後のノード）には、曲げる対象が無いので指定できない
    if (linkIndex < 0 || linkIndex >= static_cast<int>(ik->nodes.size()) - 1)
        return;

    IKJointLimit& lim = ik->limits[linkIndex];
    lim.useHinge = true;
    lim.axis = axis;
    lim.minAngle = std::min(minAngle, maxAngle);
    lim.maxAngle = std::max(minAngle, maxAngle);
}

void Animator::SetIKWeight(int ikId, float weight)
{
    IKChain* ik = GetIK(ikId);
    if (ik) ik->weight = std::clamp(weight, 0.0f, 1.0f);
}

void Animator::SetIKEnabled(int ikId, bool enabled)
{
    IKChain* ik = GetIK(ikId);
    if (ik) ik->enabled = enabled;
}

// ---------------------------------------------------------------------------
// 蝶番の角度
//   バインド姿勢からの相対回転 rel = 現在のローカル回転 × (バインドのローカル回転)^-1 を求め、
//   そのうち、軸まわりの成分（ひねり）の角度を返す。
// ---------------------------------------------------------------------------

float Animator::HingeAngleOf(int node, const IKJointLimit& limit) const
{
    const XMVECTOR axis = XMLoadFloat3(&limit.axis);
    if (XMVectorGetX(XMVector3LengthSq(axis)) < 1e-8f)
        return 0.0f;

    const XMVECTOR current = LocalRotationOf(modelData.nodes[node].localTransform);
    const XMVECTOR bind = XMLoadFloat4(&bindPoses[node].rotation);

    const Matrix rel = XMMatrixMultiply(XMMatrixRotationQuaternion(current),XMMatrixInverse(nullptr, XMMatrixRotationQuaternion(bind)));

    XMVECTOR q = XMQuaternionRotationMatrix(rel);
    if (XMVectorGetW(q) < 0.0f)
        q = XMVectorNegate(q);                       // 最短側に揃える（角度が -π〜π に収まる）

    const float s = XMVectorGetX(XMVector3Dot(q, XMVector3Normalize(axis)));   // xyz だけで内積
    return 2.0f * std::atan2(s, XMVectorGetW(q));
}

float Animator::GetIKHingeAngle(int ikId, int linkIndex) const
{
    if (ikId < 0 || ikId >= static_cast<int>(ikChains.size()))
        return 0.0f;

    const IKChain& ik = ikChains[ikId];
    if (linkIndex < 0 || linkIndex >= static_cast<int>(ik.nodes.size()) - 1)
        return 0.0f;

    return HingeAngleOf(ik.nodes[linkIndex], ik.limits[linkIndex]);
}

// ---------------------------------------------------------------------------
// ノードを、自分の位置を中心に、グローバル空間で q だけ回す
//   newGlobal = G × T(-p) × R(q) × T(p)
//   newLocal  = newGlobal × inverse(親のGlobal)
//   （行ベクトル規約。位置 p は動かず、子孫だけが動く）
// ---------------------------------------------------------------------------

void Animator::RotateNodeAroundPosition(int node, DirectX::FXMVECTOR q)
{
    const Matrix G = globalTransforms[node];
    const XMVECTOR p = G.r[3];

    const Matrix pivot =
        XMMatrixTranslationFromVector(XMVectorNegate(p)) *
        XMMatrixRotationQuaternion(q) *
        XMMatrixTranslationFromVector(p);

    const Matrix newG = XMMatrixMultiply(G, pivot);

    const int parent = modelData.nodes[node].parentIndex;
    const Matrix parentG = (parent >= 0) ? Matrix(globalTransforms[parent]) : XMMatrixIdentity();

    const Matrix local = XMMatrixMultiply(newG, XMMatrixInverse(nullptr, parentG));
    XMStoreFloat4x4(&modelData.nodes[node].localTransform, local);
}

// ---------------------------------------------------------------------------
// 2ボーンIK（余弦定理）。腕・脚向け。
//   1. 骨の長さ l1, l2 は変わらないので、付け根から目標までの距離 d が決まれば、
//      余弦定理で中間関節の位置 B' が決まる
//   2. B' が出る側（曲がる向き）は、ポール、または今の曲がり方で決める
//   3. 付け根を回して中間関節を B' へ向け、次に中間関節を回して先端を目標へ向ける
// ---------------------------------------------------------------------------

void Animator::SolveTwoBoneIK(const IKChain& ik, DirectX::FXMVECTOR target, DirectX::FXMVECTOR pole, bool hasPole)
{
    const int root = ik.nodes[0];
    const int mid  = ik.nodes[1];
    const int tip  = ik.nodes[2];

    const XMVECTOR A = globalTransforms[root].r[3];
    const XMVECTOR B = globalTransforms[mid].r[3];
    const XMVECTOR C = globalTransforms[tip].r[3];

    const float l1 = XMVectorGetX(XMVector3Length(B - A));
    const float l2 = XMVectorGetX(XMVector3Length(C - B));
    if (l1 < 1e-5f || l2 < 1e-5f)
        return;

    // 付け根から目標までの方向と距離（届く範囲に収める）
    const XMVECTOR AT = target - A;
    float d = XMVectorGetX(XMVector3Length(AT));
    if (d < 1e-5f)
        return;

    const XMVECTOR dir = AT / d;

    const float dMin = std::fabs(l1 - l2) + 1e-4f;
    const float dMax = l1 + l2 - 1e-4f;
    if (dMin > dMax)
        return;
    d = std::clamp(d, dMin, dMax);

    const XMVECTOR reachable = A + dir * d;        // 届く範囲に丸めた目標

    // 肘・膝が出る向き: 今の曲がり方 → ポールの方向へ、weight で連続的に移す
    XMVECTOR bend = RemoveComponent(B - A, dir);
    if (hasPole)
    {
        XMVECTOR toPole = RemoveComponent(pole - A, dir);
        if (XMVectorGetX(XMVector3LengthSq(toPole)) > 1e-8f && XMVectorGetX(XMVector3LengthSq(bend)) > 1e-8f)
            bend = XMVectorLerp(XMVector3Normalize(bend), XMVector3Normalize(toPole), ik.weight);
        else if (XMVectorGetX(XMVector3LengthSq(toPole)) > 1e-8f)
            bend = toPole;
    }
    if (XMVectorGetX(XMVector3LengthSq(bend)) < 1e-8f)
    {
        // 完全にまっすぐなどで向きが決まらないときは、任意の垂直方向
        const XMVECTOR up = (std::fabs(XMVectorGetY(dir)) < 0.99f) ? XMVectorSet(0, 1, 0, 0) : XMVectorSet(1, 0, 0, 0);
        bend = XMVector3Cross(dir, up);
    }
    bend = XMVector3Normalize(bend);

    // 余弦定理: 付け根での角度から、中間関節の新しい位置 B' を求める
    const float cosA = std::clamp((l1 * l1 + d * d - l2 * l2) / (2.0f * l1 * d), -1.0f, 1.0f);
    const float sinA = std::sqrt(1.0f - cosA * cosA);
    const XMVECTOR newB = A + dir * (l1 * cosA) + bend * (l1 * sinA);

    // 1 付け根を回して、中間関節を B' の方向へ向ける
    RotateNodeAroundPosition(root, RotationFromTo(B - A, newB - A));
    CalculateGlobalTransforms();

    // 2 中間関節を回して、先端を目標の方向へ向ける
    const XMVECTOR B2 = globalTransforms[mid].r[3];
    const XMVECTOR C2 = globalTransforms[tip].r[3];
    RotateNodeAroundPosition(mid, RotationFromTo(C2 - B2, reachable - B2));
    CalculateGlobalTransforms();
}

// ---------------------------------------------------------------------------
// CCD（Cyclic Coordinate Descent）。2関節以上の鎖向け（首・背骨・尻尾など）。
//   先端に近い関節から順に、「先端を目標へ向ける回転」を繰り返す。
//   ・limitAngle   : 1回に回す角度の上限（MMDのIKの「制限角」）
//   ・蝶番の関節   : 曲がる軸まわりの回転だけに絞り、可動範囲に収める（膝が逆に曲がらない）
// ---------------------------------------------------------------------------

void Animator::SolveCCD(const IKChain& ik, DirectX::FXMVECTOR target)
{
    const int count = static_cast<int>(ik.nodes.size());
    const int tip = ik.nodes.back();

    // 許容誤差の基準にするため、鎖の全長を求める
    float chainLength = 0.0f;
    for (int i = 0; i + 1 < count; ++i)
    {
        chainLength += XMVectorGetX(XMVector3Length(
            globalTransforms[ik.nodes[i + 1]].r[3] - globalTransforms[ik.nodes[i]].r[3]));
    }
    const float tolerance = std::max(ik.tolerance * chainLength, 1e-5f);

    const int iterations = std::max(ik.iterations, 1);

    for (int iter = 0; iter < iterations; ++iter)
    {
        // 先端に近い関節から、付け根へ向かって
        for (int i = count - 2; i >= 0; --i)
        {
            const int node = ik.nodes[i];
            const IKJointLimit& lim = ik.limits[i];

            const XMVECTOR J = globalTransforms[node].r[3];
            const XMVECTOR E = globalTransforms[tip].r[3];

            bool done = false;

            if (lim.useHinge)
            {
                const XMVECTOR axisLocal = XMLoadFloat3(&lim.axis);

                if (XMVectorGetX(XMVector3LengthSq(axisLocal)) > 1e-8f)
                {
                    done = true;

                    // 曲がる軸を、ワールド（ノード階層の空間）の向きへ
                    const XMVECTOR h = XMVector3Normalize(XMVector3TransformNormal(axisLocal, globalTransforms[node]));

                    // 軸に垂直な平面へ射影して、先端と目標の向きの差を、軸まわりの角度で求める
                    const XMVECTOR e = RemoveComponent(E - J, h);
                    const XMVECTOR t = RemoveComponent(target - J, h);

                    if (XMVectorGetX(XMVector3LengthSq(e)) > 1e-10f && XMVectorGetX(XMVector3LengthSq(t)) > 1e-10f)
                    {
                        float delta = std::atan2(
                            XMVectorGetX(XMVector3Dot(h, XMVector3Cross(e, t))),
                            XMVectorGetX(XMVector3Dot(e, t)));

                        if (ik.limitAngle > 0.0f)
                            delta = std::clamp(delta, -ik.limitAngle, ik.limitAngle);

                        // 可動範囲に収める
                        const float current = HingeAngleOf(node, lim);
                        const float wanted = std::clamp(current + delta, lim.minAngle, lim.maxAngle);
                        const float applied = wanted - current;

                        if (std::fabs(applied) > 1e-6f)
                            RotateNodeAroundPosition(node, XMQuaternionRotationAxis(h, applied));
                    }
                }
            }

            if (!done)
            {
                // 制限のない関節: 先端を目標へ向ける回転
                const XMVECTOR q = ClampRotationAngle(RotationFromTo(E - J, target - J), ik.limitAngle);
                RotateNodeAroundPosition(node, q);
            }

            CalculateGlobalTransforms();
        }

        const float error = XMVectorGetX(XMVector3Length(target - globalTransforms[tip].r[3]));
        if (error < tolerance)
            break;
    }
}

// ---------------------------------------------------------------------------
// 全チェーンを登録順に解く
//   目標は、weight の分だけ「今の先端（FKの結果） → 目標」へ近づけた位置を使う。
//   weight = 0 なら先端がその場に留まるため、回転は何も変わらない（連続的に切り替わる）。
// ---------------------------------------------------------------------------

void Animator::ApplyIK()
{
    if (ikChains.empty())
        return;

    // 外から受け取る位置はモデルのローカル空間。CalculateBoneMatrices で掛けている
    // globalInverse の逆行列で、ノード階層の空間へ戻す
    const Matrix toNodeSpace = XMMatrixInverse(nullptr, modelData.globalInverseTransform);

    for (const IKChain& ik : ikChains)
    {
        if (!ik.enabled || ik.weight <= 0.0f || ik.nodes.size() < 2)
            continue;

        // 目標（ノード階層の空間）
        XMVECTOR target;
        if (ik.targetNode >= 0)
            target = globalTransforms[ik.targetNode].r[3];
        else
            target = XMVector3TransformCoord(XMLoadFloat3(&ik.target), toNodeSpace);

        const float weight = std::clamp(ik.weight, 0.0f, 1.0f);
        const XMVECTOR tipPos = globalTransforms[ik.nodes.back()].r[3];
        target = XMVectorLerp(tipPos, target, weight);

        switch (ik.type)
        {
        case IKSolverType::TwoBone:
        {
            XMVECTOR pole = XMVectorZero();
            bool hasPole = false;

            if (ik.usePole)
            {
                hasPole = true;
                if (ik.poleNode >= 0)
                    pole = globalTransforms[ik.poleNode].r[3];
                else
                    pole = XMVector3TransformCoord(XMLoadFloat3(&ik.pole), toNodeSpace);
            }

            SolveTwoBoneIK(ik, target, pole, hasPole);
            break;
        }

        case IKSolverType::CCD:
            SolveCCD(ik, target);
            break;
        }
    }
}
