#pragma once
#include <string>
#include <assert.h>
#include <Windows.h>
#include "../ResultErrorHandle/ResultErrorHandle.h"
#include <filesystem>

// 拡張子を返す
inline std::wstring FileExtension(const std::wstring& path)
{
	if (path.empty()) {
		return path;
	}
	auto idx = path.rfind(L'.');

	assert(idx != std::wstring::npos);

	auto result = path.substr(idx + 1);
	return result;
}

// std::string(マルチバイト文字列)からstd::wstring(ワイド文字列)を得る。
inline std::wstring GetWideString(const std::string& str)
{
	if (str.empty())
	{
		return L"";
	}

	int num = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,str.c_str(), -1,nullptr, 0);

	if (num == 0)
	{
		return L"";
	}

	std::wstring wstr(num, L'\0');

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,str.c_str(), -1,wstr.data(), num);

	wstr.pop_back(); // 終端の L'\0' を削除

	return wstr;
}

enum class TexExtension
{
	NO = -1,
	PNG,
	JPG,
	TGA,
	PSD,
};

inline TexExtension GetExtension(const std::wstring& _filePath)
{
	std::wstring strExten = FileExtension(_filePath);

	std::wstring psd = L"psd";

	printf("文字数: %zu\n", strExten.size());

	for (wchar_t c : strExten)
	{
		printf("文字コード: 0x%04X\n",
			static_cast<unsigned int>(c));
	}

	if (strExten == L"png")
	{
		return TexExtension::PNG;
	}
	else if (strExten == L"jpg" || strExten == L"jpeg")
	{
		return TexExtension::JPG;
	}
	else if (strExten == L"tga")
	{
		return TexExtension::TGA;
	}
	else if (strExten == L"psd")
	{
		return TexExtension::PSD;
	}

	return TexExtension::NO;
}

inline TexExtension GetExtension(const std::string& _filePath)
{
	return GetExtension(GetWideString(_filePath));
}

inline ResultErrorHandle::ErrorCode CheckPath(const wchar_t* _filePath)
{
	namespace fs = std::filesystem;
	if (_filePath == nullptr || *_filePath == L'\0')
	{
		return ResultErrorHandle::ErrorCode::InvalidPath;
	}

	fs::path path(_filePath);
	std::error_code ec;

	// 親フォルダの確認
	fs::path parentPath = path.parent_path();

	if (!parentPath.empty())
	{
		bool exists = fs::exists(parentPath, ec);

		if (ec)
		{
			return ec == std::errc::permission_denied ? ResultErrorHandle::ErrorCode::AccessDenied : ResultErrorHandle::ErrorCode::InvalidPath;
		}

		if (!exists || !fs::is_directory(parentPath, ec))
		{
			return ResultErrorHandle::ErrorCode::DirectoryNotFound;
		}
	}

	// ファイルの存在確認
	bool exists = fs::exists(path, ec);

	if (ec)
	{
		return ec == std::errc::permission_denied ? ResultErrorHandle::ErrorCode::AccessDenied : ResultErrorHandle::ErrorCode::InvalidPath;
	}

	if (!exists)
	{
		return ResultErrorHandle::ErrorCode::FileNotFound;
	}

	// フォルダを指定していないか確認
	if (fs::is_directory(path, ec))
	{
		return ResultErrorHandle::ErrorCode::InvalidPath;
	}

	if (ec)
	{
		return ResultErrorHandle::ErrorCode::InvalidPath;
	}

	return ResultErrorHandle::ErrorCode::OK;
}