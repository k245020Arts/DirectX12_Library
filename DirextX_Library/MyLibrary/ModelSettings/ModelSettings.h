#pragma once
#include <string>
#include <map>
#include <array>
#include "../JsonReader/UTFConverter.h"
#include "../DirectX12Imgui/DirectX12Imgui.h"

// ============================================================================
//  ModelSettings
//  モデル(FBX)ごとの設定を「<モデルのファイル名>.json」に保存・読み込みする
//
//      data/island/Wall.fbx
//      data/island/Wall.fbx.json
//
//  ・テクスチャ名は「マテリアル名」をキーにして保存する(FBXの情報が足りなくても指定できる)
//  ・ノーマルマップ / スペキュラのパラメーターも一緒に保存する
//  ・JSON に書かれていない項目は既定値になるので、手で書くのは必要な所だけでよい
//  ・自分で足した項目(メモなど)は、プログラムが上書き保存しても消えない
//
//  【パラメーターを増やすときの手順】 例: スペキュラに "rim" を足す
//    1. 下の SpecularSettings に  float rim = 0.0f;  を足す
//    2. ModelSettings.cpp の ToJson()   に 1行足す  (書き出し)
//    3. ModelSettings.cpp の FromJson() に 1行足す  (読み込み)
//    4. FBXModel.cpp の ApplySettings() / CollectSettings() に 1行ずつ足す
// ============================================================================

// マテリアルごとのテクスチャ指定
struct MaterialSettings
{
    // モデルのフォルダからの相対パス(例: "textures/Wall_BaseColor.png")。空なら FBX の情報を使う
    std::string diffuse;
    std::string normal;

};

struct NormalMapSettings
{
    bool  enable   = true;
    float strength = 1.0f;
};

struct SpecularSettings
{
    bool  enable   = false;
    float strength = 0.5f;
    float pow      = 1.0f;
    std::array<float, 3> color = { 1.0f, 1.0f, 1.0f };
};

struct ModelSettings
{
    static constexpr int kVersion = 1;

    enum class State
    {
        New,     // JSON が無かった(これから作る)
        Loaded,  // JSON を読み込めた
        Broken,  // JSON はあるが読めなかった(上書きしない)
    };

    // ---- JSON に保存される内容 ----
    std::map<std::string, MaterialSettings> materials; // キーはマテリアル名
    NormalMapSettings normalMap;
    SpecularSettings  specular;

    // ---- 実行時だけの情報(JSON には書かない) ----
    State        state = State::New;
    bool         dirty = false;   // true なら、JSON に書き足す必要がある
    std::wstring filePath;        // JSON のパス
    std::string  rawJson;         // 読み込んだ元の JSON(自分で足された項目を残すために保持)

    // 既定値は NormalMapParam / SpecularParam(ShaderStruct.h)の初期値に合わせる
    ModelSettings();

    // モデルのパスから JSON のパスを作る  (Wall.fbx -> Wall.fbx.json)
    static std::wstring SidecarPath(const std::wstring& _modelPath);

    // 読み込む。無ければ state = New の既定値を返す。壊れていれば state = Broken
    static ModelSettings Load(const std::wstring& _modelPath);

    // 保存する。Broken のときは、_overwriteBroken が true でない限り上書きしない
    bool Save(bool _overwriteBroken = false);
};