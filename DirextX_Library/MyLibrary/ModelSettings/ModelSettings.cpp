#include "ModelSettings.h"
#include "../ShaderStruct/ShaderStruct.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <cmath>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    using Json = nlohmann::json;

    void Log(const std::wstring& _message)
    {
        OutputDebugStringW((L"[ModelSettings] " + _message + L"\n").c_str());
    }

    // 0.1f が 0.10000000149011612 と書かれないように、小数4桁で丸める
    double Round4(float _value)
    {
        return std::round(static_cast<double>(_value) * 10000.0) / 10000.0;
    }

    // ---- 読み込み用ヘルパー：キーが無ければ何もしない(= 既定値のまま) ----
    void ReadString(const Json& _j, const char* _key, std::string& _out)
    {
        auto it = _j.find(_key);
        if (it != _j.end() && it->is_string()) _out = it->get<std::string>();
    }

    void ReadFloat(const Json& _j, const char* _key, float& _out)
    {
        auto it = _j.find(_key);
        if (it != _j.end() && it->is_number()) _out = it->get<float>();
    }

    // true / false のほか、1 / 0 でも書けるようにする
    void ReadBool(const Json& _j, const char* _key, bool& _out)
    {
        auto it = _j.find(_key);
        if (it == _j.end()) return;
        if (it->is_boolean()) _out = it->get<bool>();
        else if (it->is_number()) _out = it->get<double>() != 0.0;
    }

    void ReadColor3(const Json& _j, const char* _key, std::array<float, 3>& _out)
    {
        auto it = _j.find(_key);
        if (it == _j.end() || !it->is_array() || it->size() < 3) return;
        for (size_t i = 0; i < 3; ++i)
        {
            if ((*it)[i].is_number()) _out[i] = (*it)[i].get<float>();
        }
    }

    // ============ 書き出し ============
    Json ToJson(const ModelSettings& _s)
    {
        Json j = Json::object();
        j["version"] = ModelSettings::kVersion;

        Json materials = Json::object();
        for (const auto& kv : _s.materials)
        {
            Json m = Json::object();
            m["diffuse"] = kv.second.diffuse;
            m["normal"]  = kv.second.normal;
            m["height"] = kv.second.height;
            m["metallic"] = kv.second.metallic;
            m["roughness"] = kv.second.roughness;
            m["ao"] = kv.second.ao;
            materials[kv.first] = m;
        }
        j["materials"] = materials;

        Json normal = Json::object();
        normal["enable"]   = _s.normalMap.enable;
        normal["strength"] = Round4(_s.normalMap.strength);
        j["normalMap"] = normal;

        Json height = Json::object();
        height["enable"] = _s.heightMap.enable;
        height["strength"] = Round4(_s.heightMap.strength);
        j["heightMap"] = height;

        Json specular = Json::object();
        specular["enable"]   = _s.specular.enable;
        specular["strength"] = Round4(_s.specular.strength);
        specular["pow"]      = Round4(_s.specular.pow);
        specular["color"]    = Json::array({ Round4(_s.specular.color[0]), Round4(_s.specular.color[1]), Round4(_s.specular.color[2]) });
        // ここに追加(SpecularSettings に足した項目を書き出す)
        j["specular"] = specular;

        Json bloom = Json::object();
        bloom["enable"] = _s.bloom.enable;
        bloom["intensity"] = Round4(_s.bloom.intensity);
        j["bloom"] = bloom;

        Json pbr = Json::object();

        pbr["albedo"] = Json::array({
            Round4(_s.pbr.albedo.x),
            Round4(_s.pbr.albedo.y),
            Round4(_s.pbr.albedo.z)
            });

        pbr["metallic"] = Round4(_s.pbr.metallic);
        pbr["roughness"] = Round4(_s.pbr.roughness);
        pbr["ao"] = Round4(_s.pbr.ao);

        pbr["useAlbedoMap"] = _s.pbr.useAlbedoMap;
        pbr["useMetallicMap"] = _s.pbr.useMetallicMap;
        pbr["useRoughnessMap"] = _s.pbr.useRoughnessMap;
        pbr["useAOMap"] = _s.pbr.useAOMap;

        j["pbr"] = pbr;

        return j;
    }

    // ============ 読み込み ============
    void FromJson(const Json& _j, ModelSettings& _s)
    {
        if (!_j.is_object())
        {
            throw std::runtime_error("JSON のいちばん外側がオブジェクトではありません");
        }

        auto itMat = _j.find("materials");
        if (itMat != _j.end() && itMat->is_object())
        {
            for (auto it = itMat->begin(); it != itMat->end(); ++it)
            {
                MaterialSettings m;
                if (it->is_object())
                {
                    ReadString(*it, "diffuse", m.diffuse);
                    ReadString(*it, "normal",  m.normal);
                    ReadString(*it, "height",  m.height);
                    ReadString(*it, "metallic",  m.metallic);
                    ReadString(*it, "roughness",  m.roughness);
                    ReadString(*it, "ao",  m.ao);
                }
                _s.materials[it.key()] = m;
            }
        }

        auto itNormal = _j.find("normalMap");
        if (itNormal != _j.end() && itNormal->is_object())
        {
            ReadBool (*itNormal, "enable",   _s.normalMap.enable);
            ReadFloat(*itNormal, "strength", _s.normalMap.strength);
        }

        auto itHeight = _j.find("heightMap");
        if (itHeight != _j.end() && itHeight->is_object())
        {
            ReadBool(*itHeight, "enable", _s.heightMap.enable);
            ReadFloat(*itHeight, "strength", _s.heightMap.strength);
        }

        auto itSpecular = _j.find("specular");
        if (itSpecular != _j.end() && itSpecular->is_object())
        {
            ReadBool  (*itSpecular, "enable",   _s.specular.enable);
            ReadFloat (*itSpecular, "strength", _s.specular.strength);
            ReadFloat (*itSpecular, "pow",      _s.specular.pow);
            ReadColor3(*itSpecular, "color",    _s.specular.color);
            // ここに追加(SpecularSettings に足した項目を読み込む)
        }

        auto itBloom = _j.find("bloom");
        if (itBloom != _j.end() && itBloom->is_object())
        {
            ReadBool(*itBloom, "enable", _s.bloom.enable);
            ReadFloat(*itBloom, "intensity", _s.bloom.intensity);
        }

        auto itPBR = _j.find("pbr");

        if (itPBR != _j.end() && itPBR->is_object())
        {
            // Albedo
            std::array<float, 3> albedo =
            {
                _s.pbr.albedo.x,
                _s.pbr.albedo.y,
                _s.pbr.albedo.z
            };

            ReadColor3(*itPBR, "albedo", albedo);

            _s.pbr.albedo = {albedo[0],albedo[1],albedo[2]};

            // 数値パラメーター
            ReadFloat(*itPBR, "metallic", _s.pbr.metallic);
            ReadFloat(*itPBR, "roughness", _s.pbr.roughness);
            ReadFloat(*itPBR, "ao", _s.pbr.ao);

            // テクスチャ使用設定
            ReadBool(*itPBR, "useAlbedoMap", _s.pbr.useAlbedoMap);
            ReadBool(*itPBR, "useMetallicMap", _s.pbr.useMetallicMap);
            ReadBool(*itPBR, "useRoughnessMap", _s.pbr.useRoughnessMap);
            ReadBool(*itPBR, "useAOMap", _s.pbr.useAOMap);
        }
    }

    // _need のキーが、すべて _have に存在するか(足りないキーがあれば JSON を書き足す)
    bool ContainsAllKeys(const Json& _need, const Json& _have)
    {
        if (_need.is_object())
        {
            if (!_have.is_object()) return false;
            for (auto it = _need.begin(); it != _need.end(); ++it)
            {
                auto h = _have.find(it.key());
                if (h == _have.end() || !ContainsAllKeys(*it, *h)) return false;
            }
        }
        return true;
    }
}

// ---------------------------------------------------------------------------
ModelSettings::ModelSettings()
{
    // 既定値は構造体(ShaderStruct.h)の初期値をそのまま使う。値を二重に持たないため
    const NormalMapParam normalDefault{};
    const HeightMapParam heightDefault{};
    const SpecularParam  specularDefault{};

    normalMap.enable   = normalDefault.enableNormal > 0.5f;
    normalMap.strength = normalDefault.normalStrength;

    heightMap.enable = heightDefault.enableHeight > 0.5f;
    heightMap.strength = heightDefault.heightStrength;

    specular.enable   = specularDefault.enableSpecular > 0.5f;
    specular.strength = specularDefault.specularStrength;
    specular.pow      = specularDefault.specularPow;
    specular.color    = { specularDefault.specularColor.x, specularDefault.specularColor.y, specularDefault.specularColor.z };
    // ここに追加(新しい項目の既定値。構造体に既定値があるならそれに合わせる)

    const BloomParam bloomDefault{};

    bloom.enable = bloomDefault.enableBloom > 0.5f;
    bloom.intensity = bloomDefault.bloomIntensity;

    const PBRMaterialSettings pbrMaterialDefalut{};

    pbr.albedo = pbrMaterialDefalut.albedo;
    pbr.ao = pbrMaterialDefalut.ao;
    pbr.metallic = pbrMaterialDefalut.metallic;
    pbr.roughness = pbrMaterialDefalut.roughness;
    pbr.useAlbedoMap = pbr.useAlbedoMap;
    pbr.useAOMap = pbr.useAOMap;
    pbr.useMetallicMap = pbr.useMetallicMap;
    pbr.useRoughnessMap = pbr.useRoughnessMap;
}

std::wstring ModelSettings::SidecarPath(const std::wstring& _modelPath)
{
    return _modelPath + L".json";
}

ModelSettings ModelSettings::Load(const std::wstring& _modelPath)
{
    ModelSettings s;
    s.filePath = SidecarPath(_modelPath);

    std::error_code ec;
    if (!fs::exists(fs::path(s.filePath), ec))
    {
        s.state = State::New;
        s.dirty = true;
        return s;
    }

    // 読み込めなかったときは、既定値に戻して「壊れている」扱いにする(ファイルは上書きしない)
    auto markBroken = [&](const std::wstring& _reason)
    {
        Log(s.filePath + L" を読み込めませんでした。FBX の情報で続行します。(" + _reason + L")");
        const std::wstring path = s.filePath;
        s = ModelSettings();
        s.filePath = path;
        s.state = State::Broken;
        s.dirty = false;
    };

    std::ifstream ifs(fs::path(s.filePath), std::ios::binary);
    if (!ifs)
    {
        markBroken(L"ファイルを開けません");
        return s;
    }

    const std::string text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    try
    {
        const Json j = Json::parse(text);
        FromJson(j, s);

        s.state   = State::Loaded;
        s.rawJson = text;
        // 新しく足したパラメーターなど、JSON に無い項目があれば、あとで書き足す
        s.dirty = !ContainsAllKeys(ToJson(s), j);
    }
    catch (const std::exception& e)
    {
        markBroken(UTFConverter::Utf8ToWide(e.what()));
    }

    return s;
}

bool ModelSettings::Save(bool _overwriteBroken)
{
    if (filePath.empty()) return false;

    if (state == State::Broken && !_overwriteBroken)
    {
        Log(filePath + L" は壊れているため、上書きしませんでした。JSON を直してください。");
        return false;
    }

    // 元の JSON があれば、それをベースに今の設定を重ねる(自分で足された項目を消さないため)
    Json out = Json::object();
    if (state == State::Loaded && !rawJson.empty())
    {
        try
        {
            out = Json::parse(rawJson);
        }
        catch (const std::exception&)
        {
            out = Json::object();
        }
        if (!out.is_object()) out = Json::object();
    }
    out.merge_patch(ToJson(*this));

    const std::string text = out.dump(2, ' ', false, Json::error_handler_t::replace) + "\n";

    std::ofstream ofs(fs::path(filePath), std::ios::binary | std::ios::trunc);
    if (!ofs)
    {
        Log(filePath + L" を書き込めません");
        return false;
    }
    ofs.write(text.data(), static_cast<std::streamsize>(text.size()));
    ofs.close();
    if (!ofs) return false;

    state   = State::Loaded;
    dirty   = false;
    rawJson = text;

    Log(filePath + L" を保存しました");
    return true;
}
