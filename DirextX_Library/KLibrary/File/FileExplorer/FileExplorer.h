#pragma once
#include "../FileSystemUtils/FileSystemUtils.h"
#include <list>

#define IMGUI_INCLUDE_PATH "../../../ImGui/imgui.h"	// ImGuiをインクルードするパス  プロジェクトごとにインクルードできるように、設定して下さい。

namespace FileExplorerInfo
{
	const std::string START_READ_FILEPATH = ".";	// ファイルパスを読むスタート地点
}

class FileExplorer
{
public:

	/// <summary>
	/// 初期化  初めに呼んで下さい
	/// </summary>
	static void Init();

	/// <summary>
	/// 解放  プロジェクト終了時に呼んで下さい
	/// </summary>
	static void Release();

	/// <summary>
	/// 更新　毎フレーム呼んで下さい
	/// </summary>
	static void Update();
	static void DebugView();

private:

	/// <summary>
	/// エントリデータの初期化
	/// </summary>
	static void InitEntryData();

	/// <summary>
	/// エントリの親子関係データ
	/// </summary>
	struct EntryParentChildRelation
	{
		EntryParentChildRelation* parent;
		std::list<EntryParentChildRelation*> childList;

		FileSystemUtils::EntryPathData pathData;

		EntryParentChildRelation(const FileSystemUtils::EntryPathData& _pathData, EntryParentChildRelation* _parent) : pathData(_pathData), parent(_parent), childList({}) {}

		~EntryParentChildRelation()
		{
			for (auto itr = childList.begin();itr != childList.end();)
			{
				if (*itr != nullptr)
				{
					//delete *itr;
					*itr = nullptr;
				}
				itr = childList.erase(itr);
			}
		}

		void PushChild(EntryParentChildRelation* _child)
		{
			childList.emplace_back(_child);
		}
	};

	/// <summary>
	///									フォルダサイズを初期化する
	/// </summary>
	/// <param name="_entryParentChild">エントリの親子関係データ				</param>
	/// <param name="_sumSize">			フォルダサイズを記録するuintmax_t型参照	</param>
	static void InitDirectorySize(EntryParentChildRelation* _entryParentChild,std::uintmax_t& _sumSize, bool _canAdd);

	/// <summary>
	/// エントリ作成時のImGuiWindowのデータ
	/// </summary>
	struct CreateEntryImGuiWindowData
	{
		bool isDraw;				// Windowを描画するかどうか
		std::string filePath;		// ファイルを作成するパス
		std::string fileName;		// 作成するファイル名

		CreateEntryImGuiWindowData() : isDraw(false), filePath(""), fileName("") {};

		/// <summary>
		/// エントリ関連のメンバ値のリセット
		/// </summary>
		void ResetEntryData()
		{
			isDraw = false;
			filePath.clear();
			fileName.clear();
		}
	};

	/// <summary>
	/// クラスのファイルの種類
	/// </summary>
	enum class CLASS_FILE_TYPE
	{
		NONE = 0,	// ユーザー指定のファイルタイプ
		CLASS,
		CPP,
		H,
	};

	inline static const char* FILE_TYPE_STRING_LIST[] =
	{
		u8"ユーザー指定のファイル",
		u8"クラスファイル (.cpp + .h)",
		u8"ソースファイル (.cpp)",
		u8"ヘッダーファイル (.h)"
	};

	/// <summary>
	/// ファイル作成時のImGuiWindowのデータ
	/// </summary>
	struct CreateFileImGuiWindowData : public CreateEntryImGuiWindowData
	{
		std::string fileExtension;	// 作成ファイルの拡張子
		CLASS_FILE_TYPE fileType;	// 作成するファイルタイプ　NONEの時は、拡張子をユーザーが設定できる
		std::string selectFileType;	// 選択したファイル作成の種類文字列

		/*
		/// <summary>
		///									コンストラクタ
		/// </summary>
		/// <param name="_isDraw">			Windowを描画するかどうか											</param>
		/// <param name="_filePath">		ファイルを作成するパス												</param>
		/// <param name="_fileName">		作成するファイル名													</param>
		/// <param name="_fileExtension">	作成ファイルの拡張子												</param>
		/// <param name="_type">			作成するファイルタイプ　NONEの時は、拡張子をユーザーが設定できる	</param>
		/// <param name="_selectFileType">	選択したファイル作成の種類文字列									</param>
		CreateFileImGuiWindowData(bool _isDraw, const std::string& _filePath, const std::string& _fileName, const std::string& _fileExtension,const CLASS_FILE_TYPE& _type,const std::string& _selectFileType) :
			isDraw(_isDraw), filePath(_filePath), fileName(_fileName), fileExtension(_fileExtension), fileType(_type), selectFileType(_selectFileType) {};
		*/

		CreateFileImGuiWindowData() : fileExtension(FileExplorerInfo::START_READ_FILEPATH), fileType(CLASS_FILE_TYPE::NONE), selectFileType(FILE_TYPE_STRING_LIST[0]) {};

		/// <summary>
		/// ファイル関連のメンバ値のリセット
		/// </summary>
		void ResetFileData()
		{
			ResetEntryData();
			fileExtension	= FileExplorerInfo::START_READ_FILEPATH;
			fileType		= CLASS_FILE_TYPE::NONE;
			selectFileType	= FILE_TYPE_STRING_LIST[0];
		}
	};

	/// <summary>
	/// EntryParentChildRelationの親へ子をpushする
	/// </summary>
	static void PushChildToParent();

	/// <summary>
	///									EntryParentChildRelationをツリーノードで表示する
	/// </summary>
	/// <param name="_entryParentCild">	EntryParentChildRelationポインタ	</param>
	/// <param name="_isDrawnButton">	ImGui::Buttonを描画したかどうか		</param>
	static void TreeNodeDraw(EntryParentChildRelation* _entryParentChild, bool& _isDrawnButton);

	/// <summary>
	///						コンテナ要素をツリーノードで表示する
	/// </summary>
	/// <param name="_itr">	EntryParentChildRelationイテレータ	</param>
	/// <param name="_list">EntryParentChildRelationコンテナ	</param>
	static void TreeNodeContainerDraw(std::list<EntryParentChildRelation*>::iterator& _itr, std::list<EntryParentChildRelation*>& _list);

	/// <summary>
	///											EntryParentChildRelationをコンテナにpushする
	/// </summary>
	/// <param name="_itr">						EntryParentChildRelationに入れるEntryPathDataイテレータ		</param>
	/// <param name="_parent">					親															</param>
	/// <param name="_beforeEntryParentChild">	前回コンテナにpushされたEntryParentChildRelation			</param>
	static void PushEntryParentChildRelation(std::list<FileSystemUtils::EntryPathData>::iterator& _itr, EntryParentChildRelation* _parent, EntryParentChildRelation*& _beforeEntryParentChild);

	/// <summary>
	///									ImGui::Buttonを使った関数を処理する
	/// </summary>
	/// <param name="_entryPathData">	エントリのパスデータ													</param>
	/// <returns>						true: ImGui::Buttonを表示した / false: ImGui::Buttonを表示していない	</returns>
	static bool ButtonDrawings(const FileSystemUtils::EntryPathData& _entryPathData);

	//-- コピー -----------------------------------------------

	/// <summary>
	///									Copy系のImGui::Buttonを表示する
	/// </summary>
	/// <param name="_entryPathData">	エントリのパスデータ	</param>
	static void CopyButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData);

	//-- エントリ ----------------------------------------------

	/// <summary>
	///												エントリ作成のImGui::Buttonを表示する  Buttonを押したら、エントリを作成するImGuiWindowが表示される
	/// </summary>
	/// <param name="_entryPathData">				エントリのパスデータ												</param>
	/// <param name="_buttonName">					ImGui::Buttonの名前													</param>
	/// <param name="_createEntryImGuiWindowData">	値を変更するCreateEntryImGuiWindowData								</param>
	static void CreateEntryButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData, const std::string& _buttonName, CreateEntryImGuiWindowData& _createEntryImGuiWindowData);

	//-- ディレクトリ ------------------------------------------

	/// <summary>
	///									フォルダ系のImGui::Buttonを表示する
	/// </summary>
	/// <param name="_entryPathData">	エントリのパスデータ	</param>
	//static void CreateDirectoryButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData);

	/// <summary>
	///				ディレクトリ作成のImGuiWindowの表示
	/// </summary>
	/// <returns>	true:ファイルを作成した / false:ファイルを作成していない	</returns>
	static bool CreateDirectoryWindowDraw();
	
	//-- ファイル ---------------------------------------------

	/// <summary>
	///									ファイル作成系のImGui::Buttonを表示する		
	/// </summary>
	/// <param name="_entryPathData">	エントリのパスデータ	</param>
	//static void CreateFileButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData);

	/// <summary>
	///				ファイル作成のImGuiWindowの表示
	/// </summary>
	/// <returns>	true:ファイルを作成した / false:ファイルを作成していない	</returns>
	static bool CreateFileWindowDraw();

	/// <summary>
	///								クラスファイルの作成をする (.cppと.hの)		
	/// </summary>
	/// <param name="_entryPath">	エントリパス	{例) ./data}				</param>
	/// <param name="_className">	クラス名									</param>
	/// <returns>					true:作成できた / false:作成できなかった	</returns>
	static bool CreateClassFiles(std::string _entryPath, std::string _className);

	/// <summary>
	///								.cpp / .hファイルの作成をする
	/// </summary>
	/// <param name="_entryPath">	エントリパス    {例) ./data}				</param>
	/// <param name="_cppFilesName">.cpp / .hのファイル名						</param>
	/// <param name="_type">		.cpp .hどちらを作成するか					</param>
	/// <returns>					true:作成できた / false:作成できなかった	</returns>
	static bool CreateCppFiles(std::string _entryPath, std::string _cppFilesName, const CLASS_FILE_TYPE& _type);

	//-------------------------------------------------------

	static std::list<EntryParentChildRelation*> entryParentChildRelationList;	// エントリの親子関係コンテナ
	static EntryParentChildRelation* beforeEntryParentChild;					// 前回のエントリ親子ポインタ

	static int directoryNum;		// フォルダの数
	static int fileNum;				// ファイルの数
	static int directoryAndFileNum;	// フォルダとファイルの数

	static std::list<std::string> allEntryNameList;	// 全てのエントリの名前文字列

	static CreateEntryImGuiWindowData createDirectoryImGuiWindowData;	// フォルダの作成のImGuiWindowのデータ
	static CreateFileImGuiWindowData createFileImGuiWindowData;			// ファイルの作成のImGuiWindowのデータ

	static bool isDrawEntrySize;	// エントリサイズを表示するかどうか
};
