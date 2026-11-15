#include "JsonReader.h"
#include <fstream>

namespace {
    std::string error = "no_errer";
}

bool JsonReader::Load(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        error = "ファイルが開けません。ファイル名 : " + filename;
        //my_error_assert(error);
        return false;
    }

    try {
        JSON temp;
        ifs >> temp;     // 成功したら
        data = temp;     // ここで初めて反映
    }
    catch (const JSON::parse_error& e) {
        error = std::string("パースが失敗しました。ファイル名 : : ") + e.what();
        assert(false && "パースが失敗しました。ファイル名 : : ");
        return false;
    }
    error.clear();
    return true;
}

bool JsonReader::Save(const std::string& filename, JSON& newData) const
{
    std::ofstream ofs(filename);// 書き込み用のファイルストリーム
    if (!ofs.is_open()) {
        return false;
    }
    ofs << newData.dump(2); // インデント2で整形
    return static_cast<bool>(ofs);
}
