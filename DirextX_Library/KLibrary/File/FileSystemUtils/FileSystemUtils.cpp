#include "FileSystemUtils.h"
#include <fstream>
#include <list>
#include <unordered_map>
#include <iostream>
#include <string>
#include <assert.h>

// 再帰的にエントリを文字列で返すときのインデントタイプ //
#define INDENT_TYPE_TREE (1)	// |--
#if !INDENT_TYPE_TREE
#define INDENT_TYPE_PATH(1)		// 親のパスも表示
#endif


namespace
{
	constexpr int INDENT_SIZE = 5;	// インデントの文字列のサイズ
}

int FileSystemUtils::GetDirectoryAndFileCount(const std::string_view& _directoryPath)
{
	return GetEntryCount(_directoryPath, ENTRY_TYPE::ALL);
}

int FileSystemUtils::GetRecursiveDirectoryAndFileCount(const std::string_view& _directoryPath)
{
	return GetRecursiveEntryCount(_directoryPath, ENTRY_TYPE::ALL);
}

int FileSystemUtils::GetDirectoryCount(const std::string_view& _directoryPath)
{
	return GetEntryCount(_directoryPath, ENTRY_TYPE::DIRECTORY);
}

int FileSystemUtils::GetRecursiveDirectoryCount(const std::string_view& _directoryPath)
{
	return GetRecursiveEntryCount(_directoryPath, ENTRY_TYPE::DIRECTORY);
}

int FileSystemUtils::GetFileCount(const std::string_view& _directoryPath)
{
	return GetEntryCount(_directoryPath, ENTRY_TYPE::FILE);
}

int FileSystemUtils::GetRecursiveFileCount(const std::string_view& _directoryPath)
{
	return GetRecursiveEntryCount(_directoryPath, ENTRY_TYPE::FILE);
}

bool FileSystemUtils::CreateDirectoryInCreateFile(std::string_view _directoryPath, std::string_view _fileName, std::string_view _writeString)
{
	return CreateEntry((std::string)_directoryPath, (std::string)_fileName, _writeString, ENTRY_TYPE::ALL);
}

bool FileSystemUtils::CreateEntryDirectory(std::string_view _directoryPath)
{
	return CreateEntry((std::string)_directoryPath, "", "", ENTRY_TYPE::DIRECTORY);
}

bool FileSystemUtils::CreateEntryFile(std::string_view _filePath, std::string_view _writeString)
{
	return CreateDirectoryAndFile(_filePath, _writeString);

	//return CreateEntry("", _filePath, _writeString, ENTRY_TYPE::FILE);
}

bool FileSystemUtils::FindFile(std::string_view _filePath)
{
	return FindEntry(_filePath);		// 処理内容はFindDirectoryと同じ  このクラスを使用する人がわかりやすくするため
}

bool FileSystemUtils::FindDirectory(std::string_view _fileDirectory)
{
	return FindEntry(_fileDirectory);	// 処理内容はFindFileと同じ   このクラスを使用する人がわかりやすくするため
}

std::list<std::string> FileSystemUtils::GetDirectoryAndFileNameList(std::string_view _directoryPath)
{
	std::filesystem::directory_iterator directoryItr = GetEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  
	
	return GetEntryNameList(_directoryPath, ENTRY_TYPE::ALL);
}

std::list<std::string> FileSystemUtils::GetDirectoryNameList(std::string_view _directoryPath)
{
	std::filesystem::directory_iterator directoryItr = GetEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  

	return GetEntryNameList(_directoryPath, ENTRY_TYPE::DIRECTORY);
}

std::list<std::string> FileSystemUtils::GetFileNameList(std::string_view _directoryPath)
{
	std::filesystem::directory_iterator directoryItr = GetEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  

	return GetEntryNameList(_directoryPath,ENTRY_TYPE::FILE);
}

std::list<std::string> FileSystemUtils::GetRecursiveDirectoryNameList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryNameList(_directoryPath, ENTRY_TYPE::DIRECTORY,_addIndent);
}

std::list<std::string> FileSystemUtils::GetRecursiveFileNameList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryNameList(_directoryPath,ENTRY_TYPE::FILE,_addIndent);
}

std::list<std::string> FileSystemUtils::GetRecursiveDirectoryAndFileNameList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryNameList(_directoryPath, ENTRY_TYPE::ALL,_addIndent);
}

std::list<FileSystemUtils::EntryPathData> FileSystemUtils::GetRecursiveDirectoryPathDataList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryPathDataList(_directoryPath, ENTRY_TYPE::DIRECTORY,_addIndent);
}

std::list<FileSystemUtils::EntryPathData> FileSystemUtils::GetRecursiveFilePathDataList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryPathDataList(_directoryPath, ENTRY_TYPE::FILE,_addIndent);
}

std::list<FileSystemUtils::EntryPathData> FileSystemUtils::GetRecursiveDirectoryAndFilePathDataList(std::string_view _directoryPath, bool _addIndent)
{
	return GetRecursiveEntryPathDataList(_directoryPath, ENTRY_TYPE::ALL,_addIndent);
}

std::list<std::string> FileSystemUtils::GetRecursiveEntryNameList(std::string_view _directoryPath, const ENTRY_TYPE& _type, bool _addIndent)
{
	std::list<std::string> entryNameList;	// フォルダ内のエントリ名コンテナ
	std::list<EntryPathData> entryPathDataList = std::move(GetRecursiveEntryPathDataList(_directoryPath, _type, _addIndent));	// フォルダ内のエントリ情報を、EntryPathDataとして保存するコンテナ

	for (const auto& entry : entryPathDataList)
	{
		entryNameList.emplace_back(entry.entryName);
	}
	return entryNameList;
}

std::list<FileSystemUtils::EntryPathData> FileSystemUtils::GetRecursiveEntryPathDataList(std::string_view _directoryPath, const ENTRY_TYPE& _type, bool _addIndent)
{
	std::list<EntryPathData> entryPathDataList;	// フォルダ内のエントリ情報を、EntryPathDataとして保存するコンテナ
	std::filesystem::recursive_directory_iterator recursiveDirectoryItr = GetRecursiveEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  

	std::vector<EntryData> entryDataList;	// エントリデータのコンテナ  最終的に全てのエントリデータがpushされる

	for (;recursiveDirectoryItr != std::filesystem::recursive_directory_iterator();recursiveDirectoryItr++)
	{
		const std::filesystem::directory_entry directoryEntry = *recursiveDirectoryItr;	// エントリ情報  &を付けると、下記のstd::next時に中身が変わるのでコピー
		bool canPushEntryName = true;	// コンテナに追加できるかどうか

		// 指定エントリがディレクトリだったら
		if (_type == ENTRY_TYPE::DIRECTORY)
		{
			// ファイルだったら
			if (!directoryEntry.is_directory())
			{
				canPushEntryName = false;
			}
		}
		// 指定エントリがファイルだったら
		else if (_type == ENTRY_TYPE::FILE)
		{
			// ディレクトリだったら
			if (directoryEntry.is_directory())
			{
				canPushEntryName = false;
			}
		}

		if (!canPushEntryName)
			continue;	// 追加できないとき continue

		int depth = recursiveDirectoryItr.depth();	// 階層の深さ 0 ～

		// エントリ情報のpush
		entryDataList.emplace_back(EntryData(directoryEntry, depth));
	}

	//_ インデントを追加して、エントリ名として記録 _//

	auto entryItr		= entryDataList.begin();// エントリデータイテレータ

	int beforDepth		= entryItr->depth;		// 前回の階層の深さ
	bool isNewHierarchy = false;				// 新しい階層かどうか

	std::unordered_map<int, std::list<EntryData>> depthFileDataList;	// フォルダ内の階層ごとのファイルのエントリデータコンテナ	指定エントリが全ての場合は、ファイルはフォルダの後に、コンテナにpushしたいので、このコンテナに一旦push

	int currentDepth = entryItr->depth;	// 階層の深さ 
	for (;entryItr != entryDataList.end();entryItr++)
	{
		currentDepth = entryItr->depth;
		const std::filesystem::directory_entry directoryEntry = entryItr->entry;	// エントリ情報

		if (beforDepth != currentDepth)
		{
			//_ ファイル名をコンテナに追加 _//

			for (auto depthFileDataItr = depthFileDataList.begin();depthFileDataItr != depthFileDataList.end();depthFileDataItr++)
			{
				int fileDepth = depthFileDataItr->first;	// ファイルの階層の深さ
				
				if (fileDepth <= currentDepth)
					continue;	// 現在よりも深い階層でなかったら
					
				for (auto fileDataItr = depthFileDataItr->second.begin();fileDataItr != depthFileDataItr->second.end();fileDataItr++)
				{
					// ファイル名をコンテナに追加
					PushEntryPathDataList(fileDataItr->entry, fileDataItr->depth, _addIndent, fileDataItr, depthFileDataItr->second, entryPathDataList);
				}
				// エントリのパスデータを追加したので、コンテナ内をクリア
				depthFileDataItr->second.clear();
			}
			beforDepth = currentDepth;
		}

		// 指定タイプが全てだったら
		if (_type == ENTRY_TYPE::ALL)
		{
			// ファイルだったら
			if (!directoryEntry.is_directory())
			{
				depthFileDataList[currentDepth];
				depthFileDataList[currentDepth].emplace_back(*entryItr);
				continue;	// ファイルを保存するコンテナに追加したので continue
			}
		}

		// ディレクトリ名をコンテナに追加
		PushEntryPathDataList(directoryEntry, currentDepth, _addIndent, entryItr, entryDataList, entryPathDataList);
	}

	//_ 最上階層以下のファイル名をコンテナに追加 _//

	for (auto depthFileDataItr = depthFileDataList.begin();depthFileDataItr != depthFileDataList.end();depthFileDataItr++)
	{
		int fileDepth = depthFileDataItr->first;	// ファイルの階層の深さ
		
		// 現在よりも深い階層だったら
		if (fileDepth >= currentDepth)
		{
			for (auto fileDataItr = depthFileDataItr->second.begin();fileDataItr != depthFileDataItr->second.end();fileDataItr++)
			{
				// ファイル名をコンテナに追加
				PushEntryPathDataList(fileDataItr->entry, fileDataItr->depth, _addIndent, fileDataItr, depthFileDataItr->second, entryPathDataList);
			}
			// エントリ名を追加したので、コンテナ内をクリア
			depthFileDataItr->second.clear();
		}
	}

	//_ 最上階層のファイル名をコンテナに追加 _//

	for (auto depthFileDataItr = depthFileDataList.begin();depthFileDataItr != depthFileDataList.end();depthFileDataItr++)
	{
		int fileDepth = depthFileDataItr->first;	// ファイルの階層の深さ

		for (auto fileDataItr = depthFileDataItr->second.begin();fileDataItr != depthFileDataItr->second.end();fileDataItr++)
		{
			// ファイル名をコンテナに追加
			PushEntryPathDataList(fileDataItr->entry, fileDataItr->depth, _addIndent, fileDataItr, depthFileDataItr->second, entryPathDataList);
		}
		// エントリのパスデータを追加したので、コンテナ内をクリア
		depthFileDataItr->second.clear();
	}
	return entryPathDataList;
}

void FileSystemUtils::PushEntryPathDataList(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::list<FileSystemUtils::EntryData>::iterator& _entryItr, std::list<FileSystemUtils::EntryData>& _entryDataList, std::list<FileSystemUtils::EntryPathData>& _entryPathDataList)
{
	std::string pathStr = _directoryEntry.path().string();

	// エントリのパスデータを追加
	_entryPathDataList.emplace_back(EntryPathData(pathStr, GetEntryPathString(_directoryEntry, _fileDepth, _addIndent, _entryItr, _entryDataList), _entryItr->depth, _directoryEntry.file_size(), _directoryEntry.is_directory()));
}

void FileSystemUtils::PushEntryPathDataList(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::vector<FileSystemUtils::EntryData>::iterator& _entryItr, std::vector<FileSystemUtils::EntryData>& _entryDataList, std::list<FileSystemUtils::EntryPathData>& _entryPathDataList)
{
	std::string pathStr = _directoryEntry.path().string();

	// エントリのパスデータを追加
	_entryPathDataList.emplace_back(EntryPathData(pathStr, GetEntryPathString(_directoryEntry,_fileDepth,_addIndent,_entryItr,_entryDataList), _entryItr->depth, _directoryEntry.file_size(), _directoryEntry.is_directory()));
}

std::string FileSystemUtils::GetEntryPathString(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::vector<FileSystemUtils::EntryData>::iterator& _entryItr, std::vector<FileSystemUtils::EntryData>& _entryDataList)
{
	// エントリ名の設定 //
	std::string entryName;	// エントリ名
	entryName.reserve(_directoryEntry.path().filename().string().size() + _fileDepth * INDENT_SIZE);
	if (_addIndent)
	{
		// インデント文字列を追加
		entryName += GetIndentEntryPathString(_fileDepth, _entryItr, _entryDataList);
	}
	entryName += _directoryEntry.path().filename().string().c_str();

	return entryName;
}

std::string FileSystemUtils::GetEntryPathString(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::list<FileSystemUtils::EntryData>::iterator& _entryItr, std::list<FileSystemUtils::EntryData>& _entryDataList)
{
	// エントリ名の設定 //
	std::string entryName;	// エントリ名
	entryName.reserve(_directoryEntry.path().filename().string().size() + _fileDepth * INDENT_SIZE);
	if (_addIndent)
	{
		// インデント文字列を追加
		entryName += GetIndentEntryPathString(_fileDepth, _entryItr, _entryDataList);
	}
	entryName += _directoryEntry.path().filename().string().c_str();

	return entryName;
}

std::string FileSystemUtils::GetIndentEntryPathString(const int& _depth, const std::vector<EntryData>::iterator& _entryItr, const std::vector<EntryData>& _entryDataList)
{
	std::string indentStr		= "";			// インデント文字列
	const std::string addIndentStr	= "    ";	// 追加するインデント文字列

	for (int i = 0;i < _depth;i++)
	{
		// インデントの追加
		indentStr += addIndentStr;

		if (i < (_depth - 1))
			continue;	// 最後のインデントでなかったら continue

		auto nextEntryItr = std::next(_entryItr);	// 次のエントリデータのイテレーター

		if (nextEntryItr != _entryDataList.end())
		{
			// 次の階層が異なる階層だったら文字列を' `-- 'に設定 //
			if (_depth != nextEntryItr->depth)
			{
				indentStr += "`-- ";
				continue;	// 文字列の追加をしたので continue
			}
		}
		else// 次のエントリデータがコンテナの末端
		{
			indentStr += "`-- ";
			continue;	// 文字列の追加をしたので continue
		}
		// 文字列の追加
		indentStr += "|-- ";
	}
	return indentStr;
}

std::string FileSystemUtils::GetIndentEntryPathString(const int& _depth, const std::list<EntryData>::iterator& _entryItr, const std::list<EntryData>& _entryDataList)
{
	std::string indentStr = "";		// インデント文字列
	std::string addIndentStr = "    ";	// 追加するインデント文字列

	for (int i = 0;i < _depth;i++)
	{
		// インデントの追加
		indentStr += addIndentStr;

		if (i < (_depth - 1))
			continue;	// 最後のインデントでなかったら continue

		auto nextEntryItr = std::next(_entryItr);	// 次のエントリデータのイテレーター

		if (nextEntryItr != _entryDataList.end())
		{
			// 次の階層が異なる階層だったら文字列を' `-- 'に設定 //
			if (_depth != nextEntryItr->depth)
			{
				indentStr += "`-- ";
				continue;	// 文字列の追加をしたので continue
			}
		}
		else// 次のエントリデータがコンテナの末端
		{
			indentStr += "`-- ";
			continue;	// 文字列の追加をしたので continue
		}
		// 文字列の追加
		indentStr += "|-- ";
	}
	return indentStr;
}
std::string FileSystemUtils::GetIndentEntryPathString(const int& _depth, const int& _nextDepth)
{
	std::string indentStr = "";		// インデント文字列
	std::string addIndentStr = "    ";	// 追加するインデント文字列

	for (int i = 0;i < _depth;i++)
	{
		// インデントの追加
		indentStr += addIndentStr;

		if (i < (_depth - 1))
			continue;	// 最後のインデントでなかったら continue

		if (_depth != _nextDepth)
		{
			indentStr += "`-- ";
			continue;	// 文字列の追加をしたので continue
		}

		// 文字列の追加
		indentStr += "|-- ";
	}
	return indentStr;
}

bool FileSystemUtils::CreateDirectoryAndFile(std::string_view _filePath, std::string_view _writeString)
{
	std::string directoryPath = std::string(_filePath);	// ディレクトリパス
	std::list<char> reverseFileName;					// 逆順のまま、ファイルパスの後ろから'/'までの文字列を保存するコンテナ	std::stringにpush_front()がないので、処理負荷軽減のため一度逆順でいいので、保存する
	std::string fileName;								// reverseFileNameの逆順のファイル名を元の順に戻した文字列　

	//_ ディレクトリパスの設定 _//

	// ファイルパス文字列を後ろから回す
	for (auto pathItr = _filePath.rbegin();pathItr != _filePath.rend();pathItr++)
	{
		if (*pathItr == '/')
			break;	// '/'を見つけたら break

		std::string::iterator directoryPathItr = directoryPath.end() - 1;	// 最後の文字のイテレータ

		// ディレクトリパスからファイル名を削除するために、文字列の最後を削除
		directoryPathItr = directoryPath.erase(directoryPathItr);

		// 逆順のまま、ファイルパスの後ろから「 / 」までを代入
		reverseFileName.emplace_back(*pathItr);
	}

	//_ ファイルパスの設定 _//

	// reverseFileNameで保存した逆ファイル名文字列を後ろから回す
	for (auto rFileNameItr = reverseFileName.rbegin();rFileNameItr != reverseFileName.rend();rFileNameItr++)
	{
		// 逆順から戻した、ファイルパスの後ろから「 / 」までのファイル名を代入
		fileName += *rFileNameItr;
	}

	return CreateDirectoryInCreateFile(directoryPath, fileName, _writeString);
}

bool FileSystemUtils::CreateEntry(std::string _directoryPath, std::string _fileName, std::string_view _writeString, const ENTRY_TYPE& _type)
{
	std::string entryPath = _directoryPath + _fileName;	// ファイル作成のためのファイルパス + ファイル名
	std::filesystem::path findEntry(entryPath);			// エントリのパス情報

	if (std::filesystem::exists(findEntry))
		return false;	// すでにエントリが存在したら return

	assert(!_directoryPath.empty() && "ディレクトリパスが設定されていません。");

	// ファイルを作成するタイプでなかったら
	if (_type != ENTRY_TYPE::FILE)
	{
		try
		{
			// ディレクトリの作成
			std::filesystem::create_directories(_directoryPath);
		}
		catch (std::filesystem::filesystem_error)
		{
			assert(false && "ディレクトリの作成に失敗しました。");
			return false;
		}

		// ディレクトリを作成するタイプだったら
		if (_type == ENTRY_TYPE::DIRECTORY)
		{
			return true;
		}
	}

	// ファイルの作成
	std::ofstream ofs(entryPath);

	// ファイルに書く文字列が空でないとき
	if (!_writeString.empty())
	{
		assert(_type != ENTRY_TYPE::DIRECTORY && "フォルダ内に文字列を書き込むことは出来ません。");
		assert(ofs.is_open() && "ファイルを開けませんでした。");

		// 文字列をファイルに書き込み
		ofs << _writeString << std::endl;
	}
	return (bool)ofs;
}

bool FileSystemUtils::FindEntry(std::string_view _filePath)
{
	std::filesystem::path findToPath(_filePath);

	if (std::filesystem::exists(findToPath))
		return true;	// エントリが見つかった
	return false;
}

std::filesystem::directory_iterator FileSystemUtils::GetEntry(std::string_view _directoryPath)
{
	std::error_code errorCode;														// 下記のディレクトリパスの先頭要素を指すイテレータ構築時に、エラー判定をする際に使用 valueが0でエラーなし:0以外でエラーあり
	std::filesystem::directory_iterator directoryItr(_directoryPath, errorCode);	// ディレクトリパスの先頭要素を指すイテレータを構築  

	assert(!errorCode && "ディレクトリパスの先頭要素を指すイテレータ構築に失敗しました。不正なディレクトリパスの可能性があります。");
	assert(std::filesystem::is_directory(std::filesystem::path(_directoryPath)) && "引数がディレクトリパスではありません。");

	return directoryItr;
}

std::filesystem::recursive_directory_iterator FileSystemUtils::GetRecursiveEntry(std::string_view _directoryPath)
{
	std::error_code errorCode;																		// 下記のディレクトリパスの先頭要素を指すイテレータ構築時に、エラー判定をする際に使用 valueが0でエラーなし:0以外でエラーあり
	std::filesystem::recursive_directory_iterator recursiveDirectoryItr(_directoryPath, errorCode);	// 再帰的なディレクトリパスの先頭要素を指すイテレータを構築  

	assert(!errorCode && "再帰的なディレクトリパスの先頭要素を指すイテレータ構築に失敗しました。不正なディレクトリパスの可能性があります。");
	assert(std::filesystem::is_directory(std::filesystem::path(_directoryPath)) && "引数がディレクトリパスではありません。");

	return recursiveDirectoryItr;
}

int FileSystemUtils::GetEntryCount(const std::string_view& _directoryPath, const ENTRY_TYPE& _type)
{
	std::filesystem::directory_iterator directoryItr = GetEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  
	
	return GetEntryIteratorToCount<std::filesystem::directory_iterator>(directoryItr, _type);
}

int FileSystemUtils::GetRecursiveEntryCount(const std::string_view& _directoryPath, const ENTRY_TYPE& _type)
{
	std::filesystem::recursive_directory_iterator recursiveDirectoryItr = GetRecursiveEntry(_directoryPath);	// 再帰的なディレクトリパスの先頭要素を指すイテレータを構築  
	
	return GetEntryIteratorToCount<std::filesystem::recursive_directory_iterator>(recursiveDirectoryItr, _type);
}

std::list<std::string> FileSystemUtils::GetEntryNameList(const std::string_view& _directoryPath, const ENTRY_TYPE& _type)
{
	std::list<std::string> entryNameList;											// フォルダ内のエントリ名コンテナ
	std::filesystem::directory_iterator directoryItr = GetEntry(_directoryPath);	// ディレクトリパスの先頭要素を指すイテレータを構築  

	// _directoryPath ディレクトリ内のエントリを列挙する
	for (const std::filesystem::directory_entry& directoryEntry : directoryItr)
	{
		bool canPushEntryName = false;	// コンテナに追加できるかどうか

		// 指定エントリが全てだったら
		if (_type == ENTRY_TYPE::ALL)
		{
			canPushEntryName = true;
			
		}	// 指定エントリがディレクトリだったら
		else if (_type == ENTRY_TYPE::DIRECTORY)
		{
			// ディレクトリだったら
			if (directoryEntry.is_directory())
			{
				canPushEntryName = true;
			}
		}
		else// 指定エントリがファイルだったら
		{
			// ファイルだったら
			if (!directoryEntry.is_directory())
			{
				canPushEntryName = true;
			}
		}

		if (!canPushEntryName)
			continue;	// 追加できないとき continue

		// エントリ名を追加
		entryNameList.emplace_back(directoryEntry.path().filename().string().c_str());
	}
	return entryNameList;
}
