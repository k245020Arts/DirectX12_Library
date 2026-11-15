#ifndef UTF_CONVERTER_H
#define UTF_CONVERTER_H

#ifndef _STRING_
#include <string>
#endif // _STRING_
#ifndef _VECTOR_
#include <vector>
#endif // _VECTOR_
#ifndef _LOCALE_
#include <locale>
#endif // _LOCALE_
#ifndef _CODECVT_
#include <codecvt> // for std::codecvt_utf8_utf16 (C++17で非推奨、MSVCでは利用可能)
#endif // _CODECVT_

#ifdef _WIN32
#ifndef _WINDOWS_
#include <windows.h> // Windows API for string conversion
#endif // _WINDOWS_
#endif

namespace UTFConverter
{
    // UTF-8 std::string から UTF-16 std::wstring へ変換
    inline std::wstring utf8_to_wstring(const std::string& utf8_str) {
#ifdef _WIN32
        // Windows APIを使う場合
        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
        if (wchars_num == 0) return L"";
        std::vector<wchar_t> wstr(wchars_num);
        MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wstr.data(), wchars_num);
        return std::wstring(wstr.data());
#else
        // std::codecvt_utf8_utf16 を使う場合 (C++17で非推奨だが、GCC/Clangなどではまだ使える)
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(utf8_str);
#endif
    }

    // UTF-16 std::wstring から UTF-8 std::string へ変換
    inline std::string wstring_to_utf8(const std::wstring& wstr) {
#ifdef _WIN32
        // Windows APIを使う場合
        int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8_len == 0) return "";
        std::vector<char> utf8_str(utf8_len);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8_str.data(), utf8_len, nullptr, nullptr);
        return std::string(utf8_str.data());
#else
        // std::codecvt_utf8_utf16 を使う場合
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(wstr);
#endif
    }

    inline std::wstring Utf8ToWide(const std::string& s) {
        if (s.empty()) return L"";
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        std::wstring w(n > 0 ? n - 1 : 0, L'\0');
        if (n > 0) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
        return w;
    }

    inline std::string Utf8ToSjis(const std::string& utf8) {
        std::wstring w = Utf8ToWide(utf8);
        if (w.empty() && !utf8.empty()) return {};
        BOOL used = FALSE;
        int n = WideCharToMultiByte(932, 0, w.c_str(), -1, nullptr, 0, nullptr, &used);
        std::string s(n > 0 ? n - 1 : 0, '\0');
        if (n > 0) WideCharToMultiByte(932, 0, w.c_str(), -1, &s[0], n, nullptr, &used);
        return s;
    }
}

#endif // UTF_CONVERTER_H