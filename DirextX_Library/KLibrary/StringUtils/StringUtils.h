#pragma once
#include <string>
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <assert.h>

namespace StringUtils
{
    /// <summary>
    ///                     std::string型からstd::wstring型へ変換する
    /// </summary>
    /// <param name="_str"> 変換するstd::string文字列      </param>
    /// <returns>           変換したstd::wstring文字列     </returns>
    inline std::wstring StringToWstring(const std::string& _str)
    {
        if (_str.empty())
        {
            assert(false && "文字列が空です。");
            return {};      // 文字列が空なので、return
        }

        std::wstring wstr;  // 返すstd::wstring文字列

        // 一度目の呼び出しは、文字列数を知るため
        int result = MultiByteToWideChar
        (
            CP_UTF8,
            0,
            _str.c_str(),                   // 入力文字列
            static_cast<int>(_str.size()),  // 文字列バイト数
            nullptr,
            0
        );

        assert(result > 0 && "文字列数を確認することができませんでした。");
        // 領域を確保
        wstr.resize(result);

        // 二度目の呼び出しは、変換　UTF-8からワイド文字列へ変換
        result = MultiByteToWideChar
        (
            CP_UTF8,
            0,
            _str.c_str(),                   // 入力文字列
            static_cast<int>(_str.size()),  // 文字列バイト数
            wstr.data(),                    // 変換された文字列を受け取るバッファーへのポインタ
            static_cast<int>(wstr.size())   // 変換された文字列を受け取るバッファーのサイズ
        );

        assert(result > 0 && "文字列の変換に失敗しました。");
        return wstr;
    }

    /// <summary>
    ///                     std::wstring型からstd::string型へ変換する
    /// </summary>
    /// <param name="_wstr">変換する文字列std::wstring     </param>
    /// <returns>           変換した文字列std::string      </returns>
    inline std::string WstringToString(const std::wstring& _wstr)
    {
        if (_wstr.empty())
        {
            assert(false && "文字列が空です。");
            return {};      // 文字列が空なので、return
        }

        std::string str;    // 返すstd::wstring文字列

        // 一度目の呼び出しは文字列数を知るため
        int result = WideCharToMultiByte
        (
            CP_ACP,
            0,
            _wstr.c_str(),                  // 入力文字列
            static_cast<int>(_wstr.size()), // 文字列バイト数
            nullptr,
            0,
            nullptr,
            nullptr
        );

        assert(result > 0 && "文字列数を確認することができませんでした。");
        // 領域を確保
        str.resize(result);

        // 二度目の呼び出しは、変換　ワイド文字列からマルチバイト文字列へ変換
        result = WideCharToMultiByte
        (
            CP_ACP,
            0,
            _wstr.c_str(),                  // 入力文字列
            static_cast<int>(_wstr.size()), // 文字列バイト数
            str.data(),                     // 変換された文字列を受け取るバッファーへのポインタ
            static_cast<int>(str.size()),   // 変換された文字列を受け取るバッファーのサイズ
            nullptr,
            nullptr
        );

        assert(result > 0 && "文字列の変換に失敗しました。");
        return str;
    }

    /// <summary>
    ///						    文字列をクリップボードにコピーする
    /// </summary>
    /// <param name="_text">	コピーする文字列	</param>
    inline bool CopyToClipboard(const std::string& _text)
    {
        std::wstring wText = StringToWstring(_text);            // ワイド文字列に変換した引数の文字列

        const SIZE_T charCount  = wText.size() + 1;				// 文字列のカウント（ヌル文字を含む）
        const SIZE_T byteSize   = charCount * sizeof(wchar_t);	// 文字列のサイズ

        // クリップボード用のメモリを確保する (Windows APIのメモリ確保)
        HGLOBAL hGlobalMem = GlobalAlloc(GMEM_MOVEABLE, byteSize);

        if (hGlobalMem == nullptr)
            return false;	// メモリ確保に失敗したので、return

        // メモリをロックし、ポインタを取得  (メモリをロックしないと、Windows側がメモリを移動させることがあり、wchar_tポインタを取得できない)
        wchar_t* wPointerMem = static_cast<wchar_t*>(GlobalLock(hGlobalMem));	// wchar_tポインタ

        if (wPointerMem == nullptr)
        {
            // メモリを解放する (Windows APIのメモリ開放)
            GlobalFree(hGlobalMem);
            return false;	// メモリ解放したので、return
        }

        // 文字列をコピーする
        wcscpy_s(wPointerMem, charCount, wText.c_str());

        // メモリのロックを解除する  Windows側で、動かして良いようにする
        GlobalUnlock(hGlobalMem);

        // クリップボードを開く
        if (!OpenClipboard(nullptr))
        {
            // メモリを解放する  (Windows APIのメモリ開放)
            GlobalFree(hGlobalMem);
            return false;	// メモリ解放したので、return
        }

        // クリップボードを空にする
        if (!EmptyClipboard())
        {
            // クリップボードを閉じる
            CloseClipboard();
            // メモリを解放する  (Windows APIのメモリ開放)
            GlobalFree(hGlobalMem);
            return false;   // メモリ解放したので、return
        }

        // クリップボードにデータを登録する
        bool doneCopy = (SetClipboardData(CF_UNICODETEXT, hGlobalMem) != nullptr);	// クリップボードにデータを登録できたかどうか

        // 失敗した場合
        if (!doneCopy)
        {
            // メモリを解放する (Windows APIのメモリ開放)
            GlobalFree(hGlobalMem);
        }
        // 成功した場合  CloseClipboardを呼ぶだけで、メモリ解放は、Windows側がする

        // クリップボードを閉じる
        CloseClipboard();
        return doneCopy;
    }

    /// <summary>
    ///                         数字にカンマを入れた文字列を返す
    /// </summary>
    /// <param name="_number">  数字                                  </param>
    /// <returns>               数字にカンマを入れた文字列            </returns>
    inline std::string GetNumberInCommas(const std::uintmax_t& _number)
    {
        std::string numStr          = std::to_string(_number);  // 数字文字列
        std::string reNumStr        = numStr;                   // 返す数字文字列
        long long int numStrCount   = numStr.size();            // 数字文字カウント

        while (numStrCount > 3)
        {
            numStrCount -= 3;

            // カンマの途中挿入
            reNumStr.insert(numStrCount, ",");
        }
        return reNumStr;
    }

    // ファイルのサイズ表記数値
    inline const std::uintmax_t FORMAT_FILE_SIZE[] =
    {
        1ull,                           // Byte
        1024ull,                        // KB
        1024ull * 1024,                 // MB
        1024ull * 1024 * 1024,          // GB
        1024ull * 1024 * 1024 * 1024    // TB
    };

    // ファイルのサイズ表記文字列
    const char* FORMAT_FILE_SIZE_STR[] =
    {
        "Byte",
        "KB",
        "MB",
        "GB",
        "TB"
    };

    inline std::string GetByteToFormatFileSize(const std::uintmax_t& _number)
    {
        std::uintmax_t sizeDigit = 0;   // ファイルのサイズ表記の桁

        // FORMAT_FILE_SIZEのインデックス以内かつ、_numberがファイルのサイズ表記の数値以上だったら
        while (sizeDigit + 1 < std::size(FORMAT_FILE_SIZE)
            && _number >= FORMAT_FILE_SIZE[sizeDigit + 1])
        {
            sizeDigit++;
        }

        long double formatFileSizeNum = static_cast<long double>(_number);  // ファイルのサイズ値　浮動小数点
        
        // byte単位でないとき
        if (sizeDigit > 0)
        {
            formatFileSizeNum /= FORMAT_FILE_SIZE[sizeDigit];

            // std::floor より小さい方へ数字を丸める 例) 3.9 -> 3  /  -3.7 -> -4
            // 小数第2位まで残し、それ以降を切り捨てる 例) 3.1234 -> 312.34 -> 312.00 -> 3.1200
            formatFileSizeNum = std::floor(formatFileSizeNum * 100L) / 100L;
        }

        std::ostringstream formatFileSizeOss;   // ファイルフォーマットサイズ値の数字文字列  例) 10byte  /  10.00GB
                                                // 表示する文字列をユーザーがわかりやすいように、変換するために ostringstream型を使用

        // byte単位でないとき
        if (sizeDigit > 0)
        {
            // std::fixed 小数部の桁数をより正確に指定したい場合 e での桁表示の省略をやめる
            // std::setprecision(_x) どこまで(_x文字分)、小数点以下を表示するか

            // 小数点の文字列を正確に表示 3 -> 3.00
            formatFileSizeOss << std::fixed << std::setprecision(2);
        }

        // サイズ値の追加  10
        formatFileSizeOss << formatFileSizeNum;
        // ファイルサイズ表記文字列の追加  GB
        formatFileSizeOss << FORMAT_FILE_SIZE_STR[sizeDigit];

        return formatFileSizeOss.str();
    }
}
