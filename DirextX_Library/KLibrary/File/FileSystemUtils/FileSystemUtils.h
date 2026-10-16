#pragma once
#include <filesystem>
#include <iostream>
#include <list>

// Alpha 1.0 K.H

/// <summary>
/// ファイル、ディレクトリの操作をする
/// </summary>
class FileSystemUtils
{
public:
	
	/// <summary>
	/// エントリのパスデータ
	/// </summary>
	struct EntryPathData
	{
		std::string path;		// エントリパス
		std::string entryName;	// エントリ名
		int depth;				// エントリの階層の深さ 0 ～
		std::uintmax_t size;	// エントリのサイズ byte
		bool isDirectory;		// エントリがフォルダかどうか

		/// <summary>
		///								コンストラクタ
		/// </summary>
		/// <param name="_path">		エントリパス					</param>
		/// <param name="_entryName">	エントリ名						</param>
		/// <param name="_depth">		エントリの階層の深さ 0 ～		</param>
		EntryPathData(const std::string& _path, const std::string& _entryName, const int& _depth,const std::uintmax_t& _size, bool _isDirectory) : path(_path), entryName(_entryName), depth(_depth), size(_size), isDirectory(_isDirectory) {}
	};
	
	/// <summary>
	///									フォルダパス内のフォルダとファイルの総数を返す
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス					</param>
	/// <returns>						フォルダとファイルの総数		</returns>
	static int GetDirectoryAndFileCount(const std::string_view& _directoryPath);

	/// <summary>
	///									再帰的にフォルダパス内のフォルダとファイルの総数を返す	{ 再帰的: フォルダパス内のフォルダ内部まで、見ること }
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス							</param>
	/// <returns>						再帰的なフォルダとファイルの総数		</returns>
	static int GetRecursiveDirectoryAndFileCount(const std::string_view& _directoryPath);

	/// <summary>
	///									フォルダパス内のフォルダの総数を返す
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス			</param>
	/// <returns>						フォルダの総数			</returns>
	static int GetDirectoryCount(const std::string_view& _directoryPath);

	/// <summary>
	///									再帰的にフォルダパス内のフォルダの総数を返す	{ 再帰的: フォルダパス内のフォルダ内部まで、見ること }
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス					</param>
	/// <returns>						再帰的なフォルダの総数			</returns>
	static int GetRecursiveDirectoryCount(const std::string_view& _directoryPath);

	/// <summary>
	///								フォルダパス内のファイルの総数を返す
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス			</param>
	/// <returns>						ファイルの総数			</returns>
	static int GetFileCount(const std::string_view& _directoryPath);

	/// <summary>
	///									再帰的にフォルダパス内のファイルの総数を返す	{ 再帰的: フォルダパス内のフォルダ内部まで、見ること }
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス					</param>
	/// <returns>						再帰的なファイルの総数			</returns>
	static int GetRecursiveFileCount(const std::string_view& _directoryPath);

	/// <summary>
	///									フォルダパスとファイルを作成する
	///									すでにフォルダパスが存在した場合でも、フォルダ内容が上書きされることはない。
	/// </summary>
	/// <param name="_directoryPath">	作成するフォルダパス(ディレクトリパス)	｛ 例) directory_0/ }
	///									[※最後に' / 'がないとdirectory_0と同じ階層にfile.txtが作られる]			</param>
	/// <param name="_fileName">		作成するファイル名						｛ 例) file.txt ｝					</param>
	/// <param name="_writeString">		ファイル内に書く文字列														</param>
	/// <returns>						true:成功 / false:失敗														</returns>
	static bool CreateDirectoryInCreateFile(std::string_view _directoryPath, std::string_view _fileName, std::string_view _writeString = {});

	/// <summary>
	///								フォルダパスとファイルを作成する
	///								すでにフォルダパスが存在した場合でも、フォルダ内容が上書きされることはない。
	/// </summary>
	/// <param name="_filePath">	作成するファイルパス(ディレクトリパス)	｛ 例) directory_0/file.txt ｝		</param>
	/// <param name="_writeString">	ファイル内に書く文字列														</param>
	/// <returns>					true:成功 / false:失敗														</returns>
	static bool CreateDirectoryAndFile(std::string_view _filePath, std::string_view _writeString = {});

	/// <summary>
	///									フォルダパスを作成する
	///									すでにフォルダパスが存在した場合でも、フォルダ内容が上書きされることはない。
	/// </summary>
	/// <param name="_directoryPath">	作成するフォルダパス(ディレクトリパス)	｛ 例) directory_0/directory_1 }	</param>
	/// <returns>						true:成功 / false:失敗														</returns>
	static bool CreateEntryDirectory(std::string_view _directoryPath);

	/// <summary>
	///								ファイルを作成する
	/// </summary>
	/// <param name="_filePath">	作成するファイルパス		｛ 例) directory_0/file.txt ｝	</param>
	/// <param name="_writeString">	ファイル内に書く文字列										</param>
	/// <returns>					true:成功 / false:失敗										</returns>
	static bool CreateEntryFile(std::string_view _filePath, std::string_view _writeString = {});
	
	/// <summary>
	///									ファイルパスが存在するか返す
	/// </summary>
	/// <param name="_fileDirectory">	ファイルリパス						</param>
	/// <returns>						true:存在する / false:存在しない	</returns>
	static bool FindFile(std::string_view _filePath);

	/// <summary>
	///									ディレクトリパスが存在するか返す
	/// </summary>
	/// <param name="_fileDirectory">	ディレクトリパス					</param>
	/// <returns>						true:存在する / false:存在しない	</returns>
	static bool FindDirectory(std::string_view _fileDirectory);

	/// <summary>
	///									フォルダ内のフォルダ名とファイル名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <returns>						フォルダ名とファイル名一覧						</returns>
	static std::list<std::string> GetDirectoryAndFileNameList(std::string_view _directoryPath);

	/// <summary>
	///									フォルダ内のフォルダ名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }		</param>
	/// <returns>						フォルダ名一覧							</returns>
	static std::list<std::string> GetDirectoryNameList(std::string_view _directoryPath);

	/// <summary>
	///									フォルダ内のファイル名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }		</param>
	/// <returns>						ファイル名一覧							</returns>
	static std::list<std::string> GetFileNameList(std::string_view _directoryPath);

	/// <summary>
	///									再帰的にフォルダ内のディレクトリ名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なディレクトリ名一覧						</returns>
	static std::list<std::string> GetRecursiveDirectoryNameList(std::string_view _directoryPath, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のファイル名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なファイル名一覧							</returns>
	static std::list<std::string> GetRecursiveFileNameList(std::string_view _directoryPath, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のディレクトリ名とファイル名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }					</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なディレクトリ名とディレクトリ名一覧			</returns>
	static std::list<std::string> GetRecursiveDirectoryAndFileNameList(std::string_view _directoryPath, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のディレクトリ名をEntryPathDataとしてコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なディレクトリ名一覧						</returns>
	static std::list<EntryPathData> GetRecursiveDirectoryPathDataList(std::string_view _directoryPath, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のファイル名をEntryPathDataとしてコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なファイル名一覧							</returns>
	static std::list<EntryPathData> GetRecursiveFilePathDataList(std::string_view _directoryPath, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のディレクトリ名とファイル名をEntryPathDataとしてコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なディレクトリ名とファイル名一覧			</returns>
	static std::list<EntryPathData> GetRecursiveDirectoryAndFilePathDataList(std::string_view _directoryPath, bool _addIndent);

private:

	/// <summary>
	/// エントリのタイプ
	/// </summary>
	enum class ENTRY_TYPE
	{
		ALL			= 0,	// 全て
		DIRECTORY,			// ディレクトリ
		FILE,				// ファイル
	};

	/// <summary>
	/// エントリデータ
	/// </summary>
	struct EntryData
	{
		const std::filesystem::directory_entry entry;	// エントリ情報
		int depth;	// エントリ階層の深さ 0 ～

		/// <summary>
		///							コンストラクタ
		/// </summary>
		/// <param name="_entry">	エントリ情報			</param>
		/// <param name="_depth">	エントリ階層の深さ		</param>
		EntryData(const std::filesystem::directory_entry& _entry, const int& _depth) : entry(_entry), depth(_depth) {};
	};

	/// <summary>
	///									再帰的にフォルダ内のエントリ名をコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_type">			指定エントリタイプ								</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なエントリ名一覧							</returns>
	static std::list<std::string> GetRecursiveEntryNameList(std::string_view _directoryPath, const ENTRY_TYPE& _type, bool _addIndent);

	/// <summary>
	///									再帰的にフォルダ内のエントリ名をEntryPathDataとしてコンテナで一覧を取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス｛ 例) directory_0 }				</param>
	/// <param name="_type">			指定エントリタイプ								</param>
	/// <param name="_addIndent">		インデントを追加するかどうか					</param>
	/// <returns>						再帰的なエントリ名一覧							</returns>
	static std::list<EntryPathData> GetRecursiveEntryPathDataList(std::string_view _directoryPath, const ENTRY_TYPE& _type, bool _addIndent);

	/// <summary>
	///										エントリパスデータをコンテナに追加する
	/// </summary>
	/// <param name="_directoryEntry">		エントリ情報								</param>
	/// <param name="_fileDepth">			エントリ階層の深さ 0 ～						</param>
	/// <param name="_addIndent">			インデントを追加するかどうか				</param>
	/// <param name="_entryItr">			エントリイテレータ			list::iterator	</param>
	/// <param name="_entryDataList">		エントリデータコンテナ		list			</param>
	/// <param name="_entryPathDataList">	エントリパスデータコンテナ	list			</param>
	static void PushEntryPathDataList(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::list<FileSystemUtils::EntryData>::iterator& _entryItr, std::list<FileSystemUtils::EntryData>& _entryDataList, std::list<FileSystemUtils::EntryPathData>& _entryPathDataList);

	/// <summary>
	///										エントリパスデータをコンテナに追加する
	/// </summary>
	/// <param name="_directoryEntry">		エントリ情報								</param>
	/// <param name="_fileDepth">			エントリ階層の深さ 0 ～						</param>
	/// <param name="_addIndent">			インデントを追加するかどうか				</param>
	/// <param name="_entryItr">			エントリイテレータ			vector::iterator</param>
	/// <param name="_entryDataList">		エントリデータコンテナ		vector			</param>
	/// <param name="_entryPathDataList">	エントリパスデータコンテナ	list			</param>
	static void PushEntryPathDataList(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::vector<FileSystemUtils::EntryData>::iterator& _entryItr, std::vector<FileSystemUtils::EntryData>& _entryDataList, std::list<FileSystemUtils::EntryPathData>& _entryPathDataList);

	/// <summary>
	///										エントリパスデータからエントリ名を文字列で取得する
	/// </summary>
	/// <param name="_directoryEntry">		エントリ情報								</param>
	/// <param name="_fileDepth">			エントリ階層の深さ 0 ～						</param>
	/// <param name="_addIndent">			インデントを追加するかどうか				</param>
	/// <param name="_entryItr">			エントリイテレータ			vector::iterator</param>
	/// <param name="_entryDataList">		エントリデータコンテナ		vector			</param>
	static std::string GetEntryPathString(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::vector<FileSystemUtils::EntryData>::iterator& _entryItr, std::vector<FileSystemUtils::EntryData>& _entryDataList);

	/// <summary>
	///										エントリパスデータからエントリ名を文字列で取得する
	/// </summary>
	/// <param name="_directoryEntry">		エントリ情報								</param>
	/// <param name="_fileDepth">			エントリ階層の深さ 0 ～						</param>
	/// <param name="_addIndent">			インデントを追加するかどうか				</param>
	/// <param name="_entryItr">			エントリイテレータ			list::iterator	</param>
	/// <param name="_entryDataList">		エントリデータコンテナ		list			</param>
	static std::string GetEntryPathString(const std::filesystem::directory_entry& _directoryEntry, const int& _fileDepth, bool _addIndent, std::list<FileSystemUtils::EntryData>::iterator& _entryItr, std::list<FileSystemUtils::EntryData>& _entryDataList);

	/// <summary>
	///									エントリパスをインデントして、文字列で取得する
	/// </summary>
	/// <param name="_depth">			階層の深さ							</param>
	/// <param name="_entryItr">		エントリイテレータ					</param>
	/// <param name="_entryDataList">	エントリデータコンテナ				</param>
	/// <returns>						インデントを追加した文字列を返す	</returns>
	static std::string GetIndentEntryPathString(const int& _depth, const std::vector<EntryData>::iterator& _entryItr, const std::vector<EntryData>& _entryDataList);

	/// <summary>
	///									エントリパスをインデントして、文字列で取得する
	/// </summary>
	/// <param name="_depth">			階層の深さ							</param>
	/// <param name="_entryItr">		エントリイテレータ					</param>
	/// <param name="_entryDataList">	エントリデータコンテナ				</param>
	/// <returns>						インデントを追加した文字列を返す	</returns>
	static std::string GetIndentEntryPathString(const int& _depth, const std::list<EntryData>::iterator& _entryItr, const std::list<EntryData>& _entryDataList);

	/// <summary>
	///									エントリパスをインデントして、文字列で取得する
	/// </summary>
	/// <param name="_depth">			階層の深さ							</param>
	/// <param name="_entryItr">		エントリイテレータ					</param>
	/// <param name="_entryDataList">	エントリデータコンテナ				</param>
	/// <returns>						インデントを追加した文字列を返す	</returns>
	static std::string GetIndentEntryPathString(const int& _depth, const int& _nextDepth);

	/// <summary>
	///									指定されたタイプのエントリの作成を行う
	///									
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス								</param>
	/// <param name="_fileName">		ファイル名									</param>
	/// <param name="_writeString">		ファイル内に書く文字列						</param>
	/// <param name="_type">			指定エントリタイプ							</param>
	/// <returns>						true:成功 / false:失敗						</returns>
	static bool CreateEntry(std::string _directoryPath, std::string _fileName, std::string_view _writeString, const ENTRY_TYPE& _type);

	/// <summary>
	///								エントリのパスが存在するか返す			
	/// </summary>
	/// <param name="_entryPath">	エントリパス						</param>
	/// <returns>					true:存在する / false:存在しない	</returns>
	static bool FindEntry(std::string_view _filePath);

	/// <summary>
	///									エントリ情報をイテレーターで取得する　
	///									(ディレクトリパス内のディレクトリ内部までは、再帰的に取得せずに、ディレクトリ名だけ取得)
	/// </summary>
	/// <param name="_directoryPath">	ディレクトリパス														</param>
	/// <returns>						エントリ情報のイテレーター	std::filesystem::directory_iterator			</returns>
	static std::filesystem::directory_iterator GetEntry(std::string_view _directoryPath);

	/// <summary>
	///									再帰的エントリ情報をイテレーターで取得する
	/// </summary>
	/// <param name="_directoryPath">	ディレクトリパス																	</param>
	/// <returns>						再帰的エントリ情報のイテレーター	std::filesystem::recursive_directory_iterator	</returns>
	static std::filesystem::recursive_directory_iterator GetRecursiveEntry(std::string_view _directoryPath);

	/// <summary>
	///									指定されたタイプのフォルダパス内のエントリの総数を返す
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス												</param>
	/// <param name="_type">			指定エントリタイプ 											</param>
	/// <returns>						指定されたエントリタイプのフォルダパス内のエントリの総数	</returns>
	static int GetEntryCount(const std::string_view& _directoryPath, const ENTRY_TYPE& _type);

	/// <summary>
	///									再帰的に、指定されたタイプのフォルダパス内のエントリを総数を返す
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス														</param>
	/// <param name="_type">			指定エントリタイプ 													</param>
	/// <returns>						再帰的に、指定されたエントリタイプのフォルダパス内のエントリの総数	</returns>
	static int GetRecursiveEntryCount(const std::string_view& _directoryPath, const ENTRY_TYPE& _type);
	
	/// <summary>	
	///									指定されたエントリタイプのカウント数を返す
	/// </summary>
	/// <param name="_directoryItr">	エントリイテレータ		</param>
	/// <param name="_type">			指定エントリタイプ 		</param>
	/// <returns>						エントリのカウント数	</returns>
	template<typename T,std::enable_if_t<std::is_same_v<T, std::filesystem::directory_iterator> || std::is_same_v<T, std::filesystem::recursive_directory_iterator>,std::nullptr_t> = nullptr>
	static int GetEntryIteratorToCount(const T _directoryItr, const ENTRY_TYPE _type)
	{
		int entryCount = 0;    // エントリのカウント

		// _filePathディレクトリ内のエントリを列挙する
		for (const std::filesystem::directory_entry& directoryEntry : _directoryItr)
		{
			// 全てのエントリをカウントするタイプだったら
			if (_type == ENTRY_TYPE::ALL)
			{
				// エントリ数をカウント
				entryCount++;
				continue;	// エントリ数をカウントしたのでcontinue
			}

			// ディレクトリをカウントするタイプだったら
			if (_type == ENTRY_TYPE::DIRECTORY)
			{
				// ディレクトリだったら
				if (directoryEntry.is_directory())
				{
					// エントリ数をカウント
					entryCount++;
					continue;	// エントリ数をカウントしたのでcontinue
				}
			}
			else
			{
				// ファイルだったら
				if (!directoryEntry.is_directory())
				{
					// エントリ数をカウント
					entryCount++;
					continue;	// エントリ数をカウントしたのでcontinue
				}
			}
		}
		return entryCount;
	}

	/// <summary>
	///									指定されたタイプのフォルダパス内のエントリの名前をコンテナで一覧取得する
	/// </summary>
	/// <param name="_directoryPath">	フォルダパス													</param>
	/// <param name="_type">			指定エントリタイプ 												</param>
	/// <returns>						エントリの名前コンテナ											</returns>
	static std::list<std::string> GetEntryNameList(const std::string_view& _directoryPath, const ENTRY_TYPE& _type);
};

