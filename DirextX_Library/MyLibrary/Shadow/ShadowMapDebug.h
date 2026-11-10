#pragma once

#include <memory>

class ShadowMap;
class RootSignature;
class PipelineState;

// ShadowMapの深度値をグレースケールで表示するデバッグ描画クラス。
class ShadowMapDebug
{
public:
    ShadowMapDebug();
    ~ShadowMapDebug();

    bool Init();
    bool IsValid() const;
    void Draw(const ShadowMap& shadowMap);

private:
    std::unique_ptr<RootSignature> rootSignature;
    std::unique_ptr<PipelineState> pipelineState;
};
