#include "DistortionPostEffect.h"
#include "../../Engine/Engine.h"
#include "../../RootSignature/RootSignature.h"
#include "../../PipelineState/PipelineState.h"
#include "../../DescriptorHeap/DescriptorHeap.h"
#include "../../ConstBuffer/ConstBuffer.h"
#include "../../RenderTarget/RenderTarget.h"
#include "../../Texture/TextureLoader.h"

DistortionPostEffect::DistortionPostEffect()
{
}

DistortionPostEffect::~DistortionPostEffect()
{
    descriptorHandle = nullptr;
}

ResultErrorHandle DistortionPostEffect::Load(const std::string& _path)
{
    ResultErrorHandle::ErrorCode result = TextureLoader::GetInstance()->Load(_path);

    ResultErrorHandle handle;
    handle.SetErrorCode(result);

    TextureData* texture = TextureLoader::GetInstance()->Get(_path);

    if (texture == nullptr) {
        std::string error = _path + "こちらのパスがありません";
        //assert(false &&  error.c_str());
        return handle;
    }

    descriptorHandle = DescriptorHeap::GetInstance()->Register(TextureLoader::GetInstance(), _path);

    if (descriptorHandle == nullptr)
    {
        assert(false && "Descriptorの登録に失敗しました");
        return handle;
    }

    return handle;
}

bool DistortionPostEffect::Init()
{
    auto engine = Engine::GetInstance();

    auto size = engine->GetWindowSize();

    rootSignature = std::make_unique<RootSignature>();

    rootSignature->SetDistortionRootSignature();

    pipelineState = std::make_unique<PipelineState>(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    pipelineState->SetInputLayout(D3D12_INPUT_LAYOUT_DESC());
    pipelineState->SetRootSignature(rootSignature->Get());
    pipelineState->SetDepthEnable(false);
    pipelineState->SetDrawLayOut(D3D12_FILL_MODE_SOLID);
    pipelineState->SetVS(L"Shader/PostEffectBaseShader/PostEffectBaseVertexShader.hlsl", "PostEffectBaseVSShader");
    pipelineState->SetPS(L"Shader/PostEffectBaseShader/DistortionShader/DistortionPixelShader.hlsl", "DistortionPixelShader");
    pipelineState->Create();

    if (!pipelineState->IsSuccess())
    {
        OutputDebugStringW(L"パイプラインステートの生成に失敗\n");
    }

    return true;
}

void DistortionPostEffect::Draw()
{
    if (inputTexture == nullptr)
    {
        return;
    }

    auto commandList = Engine::GetInstance()->CommandList();

    auto heap = DescriptorHeap::GetInstance()->GetHeap();

    auto currentIndex = Engine::GetInstance()->CurrentBackBufferIndex(); // 現在のフレーム番号を取得する

    ID3D12DescriptorHeap* heaps[] =
    {
        heap.Get()
    };

    commandList->SetDescriptorHeaps(1, heaps);

    commandList->SetGraphicsRootSignature(rootSignature->Get());
    //commandList->SetGraphicsRootConstantBufferView(0, postEffectConstBuff[currentIndex]->GetAddress());

    commandList->SetPipelineState(pipelineState->GetPipelineState());

    commandList->SetGraphicsRootDescriptorTable(0, inputTexture->handleGPU);
    commandList->SetGraphicsRootDescriptorTable(1, descriptorHandle->handleGPU);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);
}
