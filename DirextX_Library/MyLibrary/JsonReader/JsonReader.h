#pragma once
#include  <vector>
#include <string>
#include <nlohmann/json.hpp>

using JSON = nlohmann::json;

class JsonReader
{
public:
    /// <summary>
    /// data_に読み込む
    /// </summary>
    /// <param name="filename">ファイルネーム</param>
    /// <returns>取得できたかどうか</returns>
    bool Load(const std::string& filename);
    /// <summary>
    /// 上書き保存
    /// </summary>
    /// <param name="filename">ファイルネーム</param>
    /// <returns>セーブできたかどうか</returns>
    bool Save(const std::string& filename, JSON& newData) const;

   /// <summary>
   /// jsonデータの取得
   /// </summary>
   /// <returns>読み書き用のアクセサ</returns>
   JSON& Data(){ return data;}

private:
    JSON data;

};