#include "FileExplorer.h"
#include IMGUI_INCLUDE_PATH
#include "../../StringUtils/StringUtils.h"
#include <Windows.h>
#include <string>
#include <cwchar>
#include "../../../MyLibrary/VisualStudioAdd/VisualStudioAdd.h"

// newした際にメモリリークしたかをおうためのマクロ

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#endif


std::list<FileExplorer::EntryParentChildRelation*> FileExplorer::entryParentChildRelationList	= {};
FileExplorer::EntryParentChildRelation* FileExplorer::beforeEntryParentChild					= nullptr;
int FileExplorer::directoryNum			= 0;
int FileExplorer::fileNum				= 0;
int FileExplorer::directoryAndFileNum	= 0;
std::list<std::string> FileExplorer::allEntryNameList = {};
FileExplorer::CreateEntryImGuiWindowData FileExplorer::createDirectoryImGuiWindowData	= FileExplorer::CreateEntryImGuiWindowData();
FileExplorer::CreateFileImGuiWindowData FileExplorer::createFileImGuiWindowData			= FileExplorer::CreateFileImGuiWindowData();
bool FileExplorer::isDrawEntrySize		= false;

void FileExplorer::Init()
{
	directoryNum		= FileSystemUtils::GetRecursiveDirectoryCount("data");
	fileNum				= FileSystemUtils::GetRecursiveFileCount("data");
	directoryAndFileNum = FileSystemUtils::GetRecursiveDirectoryAndFileCount("data");

	allEntryNameList.clear();
	allEntryNameList = std::move(FileSystemUtils::GetRecursiveDirectoryAndFileNameList(FileExplorerInfo::START_READ_FILEPATH, true));
	VisualStudioProjectEditor::ProjectFilePath(L"DirextX_Library.vcxproj");
	// エントリデータの初期化
	InitEntryData();
}

void FileExplorer::InitEntryData()
{
	for (auto itr = entryParentChildRelationList.begin();itr != entryParentChildRelationList.end();)
	{
		if (*itr != nullptr)
		{
			delete* itr;
			*itr = nullptr;
		}
		itr = entryParentChildRelationList.erase(itr);
	}
	entryParentChildRelationList.clear();

	std::list<FileSystemUtils::EntryPathData> allEntryPathDataList = std::move(FileSystemUtils::GetRecursiveDirectoryAndFilePathDataList(FileExplorerInfo::START_READ_FILEPATH, false));	// 全てのエントリデータのコンテナ
	std::list<FileExplorer::EntryParentChildRelation*> topDepthEntryParentChildRelationList;	// 最上階の親子関係データのリスト
	int beforeDepth = -1;	// 前回の階層深度

	// 親子関係の初期化 //
	for (auto itr = allEntryPathDataList.begin();itr != allEntryPathDataList.end();itr++)
	{
		// 最上階層の時
		if (itr->depth == 0)
		{
			// EntryParentChildRelationをコンテナにpushする
			PushEntryParentChildRelation(itr, nullptr, beforeEntryParentChild);
			beforeDepth = itr->depth;

			// 最上階の親子関係データをコンテナにpushする
			topDepthEntryParentChildRelationList.emplace_back(beforeEntryParentChild);
			continue;	// 最上階層のエントリをpushしたので continue	
		}

		// 階層が深まった時
		if (beforeDepth < itr->depth)
		{
			// EntryParentChildRelationをコンテナにpushする
			PushEntryParentChildRelation(itr, beforeEntryParentChild, beforeEntryParentChild);

			// 親へ子をpush
			PushChildToParent();
		}
		else // 階層が浅くなった時
		{
			auto copyItr = std::prev(entryParentChildRelationList.end());	// 末尾のイテレーターのコピー

			// 一つ上の階層の親を探して、代入 //
			while (true)
			{
				EntryParentChildRelation* copyEntry = *copyItr;

				if (copyEntry->pathData.depth != itr->depth - 1)
				{
					if (copyItr == entryParentChildRelationList.begin())
						break;	// イテレーターの先頭だったら break

					--copyItr;
					continue;	// copyEntryが一つ上の階層でなかったら continue
				}

				// EntryParentChildRelationをコンテナにpushする
				PushEntryParentChildRelation(itr, copyEntry, beforeEntryParentChild);

				// 親へ子をpush
				PushChildToParent();
				break;	// 親に子をpushしたので break
			}
		}
		beforeDepth = itr->depth;
	}
	
	// ディレクトリサイズの初期化 //
	for (auto& entryParentChildRelation : entryParentChildRelationList)
	{
		InitDirectorySize(entryParentChildRelation, entryParentChildRelation->pathData.size, entryParentChildRelation->pathData.isDirectory);
	}
}

void FileExplorer::PushChildToParent()
{
	for (const auto& parent : entryParentChildRelationList)
	{
		if (beforeEntryParentChild->parent != parent)
			continue;	// 親でなかったら continue

		// 親に子をpush
		parent->PushChild(beforeEntryParentChild);
	}
}

void FileExplorer::InitDirectorySize(EntryParentChildRelation* _entryParentChild, std::uintmax_t& _sumSize, bool _canAdd)
{
	if(_canAdd)
	{
		// サイズの加算
		_sumSize += _entryParentChild->pathData.size;
	}

	if (_entryParentChild->childList.empty())
		return;	// 子がいなかったら return

	// 子のディレクトリもサイズ初期化 //
	for (auto& entryCild : _entryParentChild->childList)
	{
		InitDirectorySize(entryCild, _sumSize, _canAdd);
	}
}

void FileExplorer::Release()
{
	for (auto itr = entryParentChildRelationList.begin();itr != entryParentChildRelationList.end();)
	{
		if (*itr != nullptr)
		{
			delete *itr;
			*itr = nullptr;
		}
		itr = entryParentChildRelationList.erase(itr);
	}
	entryParentChildRelationList.clear();
	allEntryNameList.clear();

	FileExplorer::beforeEntryParentChild	= nullptr;
	FileExplorer::directoryNum				= 0;
	FileExplorer::fileNum					= 0;
	FileExplorer::directoryAndFileNum		= 0;
}

void FileExplorer::Update()
{
	//Release();
	//Init();

	ImGui::Begin("Entry");
	// フォルダ数の表示
	ImGui::Text(u8"フォルダ数 : %d", directoryNum);
	// ファイル数の表示
	ImGui::Text(u8"ファイル数 : %d", fileNum);

	ImGui::Text(u8"エントリのTreeNode --------------------------------");

	if (ImGui::Button(u8"再読み込み"))
	{
		// エントリデータの情報を再初期化
		InitEntryData();
	}
	
	ImGui::SameLine();
	
	ImGui::Text("|");

	ImGui::SameLine();

	ImGui::Checkbox(u8"ファイルサイズの表示", &isDrawEntrySize);


	// 全てのエントリ名の表示 //
	/*for (const auto& entryName : allEntryNameList)
	{
		ImGui::Text(entryName.c_str());
	}*/

	auto entryPathDataItr = entryParentChildRelationList.begin();

	// ツリーノードで、エントリを表示
	TreeNodeContainerDraw(entryPathDataItr, entryParentChildRelationList);

	ImGui::Text("------------------------------------------------");

	ImGui::End();

	bool isCreateEntry = false;	// エントリを作成したかどうか

	if (createDirectoryImGuiWindowData.isDraw)
	{
		// ディレクトリ作成のImGuiWindowの表示
		isCreateEntry |= CreateDirectoryWindowDraw();
	}

	if (createFileImGuiWindowData.isDraw)
	{
		// ファイル作成のImGuiWindowの表示
		isCreateEntry |= CreateFileWindowDraw();
	}

	if (isCreateEntry)
	{
		// エントリを作成したら、エントリデータの情報を再初期化
		InitEntryData();
	}
}

void FileExplorer::DebugView()
{

}

void FileExplorer::TreeNodeDraw(EntryParentChildRelation* _entryParentChild, bool& _isDrawnButton)
{
	FileSystemUtils::EntryPathData& entryPathData = _entryParentChild->pathData;

	std::string nodeName = entryPathData.entryName;	// TreeNodeの名前
	if (isDrawEntrySize)
	{
		// エントリ名の横に、エントリサイズの文字列を追加
		nodeName += "	 | " + StringUtils::GetByteToFormatFileSize(entryPathData.size) + "      (" + StringUtils::GetNumberInCommas(entryPathData.size) + "byte) ";
	}
	float windowRightX			= ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;		// ウィンドウ領域の右端
	ImVec2 treeNodeRowMinPos	= ImGui::GetCursorScreenPos();											// TreeNodeの行座標左上
	ImVec2 treeNodeRowMaxPos	= ImVec2(windowRightX, treeNodeRowMinPos.y + ImGui::GetFrameHeight());	// TreeNodeの行座標右下
	bool isTreeNodeHovered		= (ImGui::IsMouseHoveringRect(treeNodeRowMinPos, treeNodeRowMaxPos) && !_isDrawnButton);// マウスポインタがTreeNode上にあるかどうか

	// ファイル作成のImGuiWindowが表示されていたら、他のファイル関連の操作は行わない
	/*if (createFileImGuiWindowData.isDraw)
	{
		isTreeNodeHovered = false;
	}*/

	if (_entryParentChild->childList.empty())
	{
		if (ImGui::TreeNodeEx(nodeName.c_str(), ImGuiTreeNodeFlags_Leaf))
		{
			ImGui::TreePop();
		}

#if 0
		// TreeNodeの判定範囲を取得
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();

		// 赤い枠を表示
		ImGui::GetWindowDrawList()->AddRect(
			min,
			max,
			IM_COL32(255, 0, 0, 255)
		);
#endif

		if (isTreeNodeHovered)
		{
			// ImGui::Button表示
			_isDrawnButton = ButtonDrawings(entryPathData);
		}
		return;	// 子が存在しないので return
	}

	bool isTreeNodeExOpen = ImGui::TreeNodeEx(nodeName.c_str());	// TreeNodeを開いているかどうか

#if 0
	// TreeNodeの判定範囲を取得
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();

	// 赤い枠を表示
	ImGui::GetWindowDrawList()->AddRect(
		min,
		max,
		IM_COL32(255, 0, 0, 255)
	);
#endif

	if (isTreeNodeExOpen)
	{
		if (isTreeNodeHovered)
		{
			// ImGui::Button表示
			_isDrawnButton = ButtonDrawings(entryPathData);
		}

		for (const auto& entryCild : _entryParentChild->childList)
		{
			TreeNodeDraw(entryCild, _isDrawnButton);
		}
		ImGui::TreePop();
		return;	// ツリーノードの最後でなかったら、以下のImGui::Buttonをスキップする  上記で表示したため
	}

	if (isTreeNodeHovered)
	{
		// ImGui::Button表示
		_isDrawnButton = ButtonDrawings(entryPathData);
	}
}

void FileExplorer::TreeNodeContainerDraw(std::list<EntryParentChildRelation*>::iterator& _itr, std::list<EntryParentChildRelation*>& _list)
{
	bool isDrawnButton = false;	// ImGui::Buttonを描画したかどうか
	
	for (const auto& entry : _list)
	{
		if (entry->pathData.depth != 0)
			continue;	// 最上階でないとき continue

		// 再帰的関数でエントリ情報の表示
		TreeNodeDraw(entry, isDrawnButton);
	}
}

void FileExplorer::PushEntryParentChildRelation(std::list<FileSystemUtils::EntryPathData>::iterator& _itr, EntryParentChildRelation* _parent, EntryParentChildRelation*& _beforeEntryParentChild)
{
	EntryParentChildRelation* entryParentChild = new EntryParentChildRelation(*_itr, _parent);
	entryParentChildRelationList.emplace_back(entryParentChild);

	_beforeEntryParentChild = entryParentChild;
}

bool FileExplorer::ButtonDrawings(const FileSystemUtils::EntryPathData& _entryPathData)
{
	// コピーのImGui::Buttonの表示
	CopyButtonDraw(_entryPathData);

	if (!_entryPathData.isDirectory)
		return true;	// エントリがフォルダでなかったら、以下の作成処理をせず、return

	if (!createDirectoryImGuiWindowData.isDraw)
	{
		// フォルダ作成のImGui::Buttonの表示
		CreateEntryButtonDraw(_entryPathData, u8"フォルダの作成", createDirectoryImGuiWindowData);
	}

	if (!createFileImGuiWindowData.isDraw)
	{
		// ファイル作成のImGui::Buttonの表示
		CreateEntryButtonDraw(_entryPathData, u8"ファイルの作成", createFileImGuiWindowData);
	}
	return true;
}

//-- コピー -----------------------------------------------

void FileExplorer::CopyButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData)
{
	std::string buttonEntryNameStr = u8"名前コピー";	// ImGui::Buttonの引数文字列(エントリ名)
	std::string buttonEntryPathStr = u8"パスコピー";	// ImGui::Buttonの引数文字列(エントリパス名)

	ImVec2 buttonSize	= ImVec2(0.0f, 18.0f);		// ImGui::ButtonのUIサイズ  Xは0にすることで、デフォルトの値をを使うようにする

	ImGui::SameLine();
	
	float treeNodeRight = ImGui::GetItemRectMax().x;// TreeNodeの右端

	// 以下のButton表示初め位置を設定
	ImGui::SetCursorScreenPos(ImVec2(treeNodeRight + 10.0f, ImGui::GetCursorScreenPos().y));

	// Buttonの表示文字位置を左上に設定
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));

	if (ImGui::Button(buttonEntryNameStr.c_str(), buttonSize))
	{
		// 文字列をクリップボードにコピーする
		StringUtils::CopyToClipboard(_entryPathData.entryName);
	}

	ImGui::PopStyleVar();

#if 1
	ImVec2 buttonMin = ImGui::GetItemRectMin();
	ImVec2 buttonMax = ImGui::GetItemRectMax();

	ImGui::GetWindowDrawList()->AddRect(
		buttonMin,
		buttonMax,
		IM_COL32(0, 255, 0, 255)
	);
#endif

	ImGui::SameLine();

	if (ImGui::Button(buttonEntryPathStr.c_str(), buttonSize))
	{
		std::string pathStr = _entryPathData.path.c_str();	// パスの文字列

		//_ 不必要な先頭文字列の削除 _//

		const size_t MAX_ERASE_STR_COUNT = FileExplorerInfo::START_READ_FILEPATH.size() + 1;	// 削除する最大の文字カウント数  .と/を合わせた文字数(2文字)
		int eraseCount = 0;	// 削除カウント

		for (auto pathItr = pathStr.begin();pathItr != pathStr.end();)
		{
			pathItr = pathStr.erase(pathItr);

			if (++eraseCount >= MAX_ERASE_STR_COUNT)
				break;	// 削除カウントが最大になったら break
		}

		//_ \を/に変換する _//

		for (auto pathItr = pathStr.begin();pathItr != pathStr.end();)
		{
			/* \の時  プログラム表記では\\ */
			if (*pathItr == '\\')
			{
				pathItr = pathStr.erase(pathItr);
				pathStr.insert(pathItr, '/');
				continue;	// 削除し文字を追加したので continue
			}
			pathItr++;
		}
		// 文字列をクリップボードにコピーする
		StringUtils::CopyToClipboard(pathStr);
	}

#if 1
	buttonMin = ImGui::GetItemRectMin();
	buttonMax = ImGui::GetItemRectMax();

	ImGui::GetWindowDrawList()->AddRect(
		buttonMin,
		buttonMax,
		IM_COL32(0, 255, 0, 255)
	);
#endif
}

//-- エントリ -----------------------------------------------

void FileExplorer::CreateEntryButtonDraw(const FileSystemUtils::EntryPathData& _entryPathData, const std::string& _buttonName, CreateEntryImGuiWindowData& _createEntryImGuiWindowData)
{
	ImVec2 buttonSize = ImVec2(0.0f, 18.0f);			// ImGui::ButtonのUIサイズ  Xは0にすることで、デフォルトの値をを使うようにする

	ImGui::SameLine();

	float treeNodeRight = ImGui::GetItemRectMax().x;// TreeNodeの右端

	// 以下のButton表示初め位置を設定
	ImGui::SetCursorScreenPos(ImVec2(treeNodeRight + 10.0f, ImGui::GetCursorScreenPos().y));

	// Buttonの表示文字位置を左上に設定
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));

	if (ImGui::Button(_buttonName.c_str(), buttonSize))
	{
		_createEntryImGuiWindowData.isDraw = true;
		_createEntryImGuiWindowData.filePath = _entryPathData.path;
	}

	ImGui::PopStyleVar();

#if 1
	ImVec2 buttonMin = ImGui::GetItemRectMin();
	ImVec2 buttonMax = ImGui::GetItemRectMax();

	ImGui::GetWindowDrawList()->AddRect(
		buttonMin,
		buttonMax,
		IM_COL32(0, 255, 0, 255)
	);
#endif
}

//-- ディレクトリ -------------------------------------------

bool FileExplorer::CreateDirectoryWindowDraw()
{
	bool isCreateDirectory = false;	// ディレクトリ作成したかどうか

	ImGui::Begin(u8"フォルダ作成");

	ImGui::Text(u8"追加フォルダパス : %s", createDirectoryImGuiWindowData.filePath.c_str());

	static char fileNameBuffer[256];// 設定するディレクトリ文字列

	// ディレクトリ名の入力
	if (ImGui::InputText(u8"名前", fileNameBuffer, 256))
	{
		createDirectoryImGuiWindowData.fileName = fileNameBuffer;
	}

	// ディレクトリ名が設定されているとき
	if (!createDirectoryImGuiWindowData.fileName.empty())
	{
		if (ImGui::Button(u8"追加"))
		{
			std::string directoryPath = createDirectoryImGuiWindowData.filePath;	// ディレクトリパス
			directoryPath += "/";
			directoryPath += createDirectoryImGuiWindowData.fileName;

			// ディレクトリの作成
			isCreateDirectory = FileSystemUtils::CreateEntryDirectory(directoryPath);

			// 文字列をリセット
			strcpy_s(fileNameBuffer, "");

			// ディレクトリ名文字列のリセット
			createDirectoryImGuiWindowData.fileName.clear();
		}
	}

	if (ImGui::Button(u8"キャンセル"))
	{
		// 文字列をリセット
		strcpy_s(fileNameBuffer, "");

		// ディレクトリ関連のメンバ値のリセット
		createDirectoryImGuiWindowData.ResetEntryData();
		ImGui::End();
		return isCreateDirectory;	// Window表示をOFFにしたので、return
	}

	ImGui::End();

	return isCreateDirectory;
}

//-- ファイル ---------------------------------------------

bool FileExplorer::CreateFileWindowDraw()
{
	bool isCreateFile = false;	// ファイルを作成したかどうか

	ImGui::Begin(u8"ファイル作成");

	ImGui::Text(u8"追加ファイルパス : %s", createFileImGuiWindowData.filePath.c_str());

	bool isSelectList = false;	// ImGui::Comboのリストを選択したかどうか
	int index = 0;

	// ImGui::Comboでファイルのタイプをリスト内から選択 //
	if (ImGui::BeginCombo("Type", createFileImGuiWindowData.selectFileType.c_str()))
	{
		for (index;index < std::size(FILE_TYPE_STRING_LIST);index++)
		{
			if (ImGui::Selectable(FILE_TYPE_STRING_LIST[index]))
			{
				createFileImGuiWindowData.selectFileType	= FILE_TYPE_STRING_LIST[index];
				createFileImGuiWindowData.fileType			= (CLASS_FILE_TYPE)index;
				isSelectList = true;
			}
		}
		ImGui::EndCombo();
	}

	static char fileExtensionBuffer[256];	// 設定する拡張子文字列
	static char fileNameBuffer[256];		// 設定するファイル文字列

	if (createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::NONE)
	{
		// Comboの表示位置を取得 //
		ImVec2 comboMin = ImGui::GetItemRectMin();
		ImVec2 comboMax = ImGui::GetItemRectMax();

		// 「.」の表示位置
		ImVec2 dotPos = ImVec2(comboMin.x, comboMax.y + 11.0f);

		ImGui::SetCursorScreenPos(dotPos);
		
		ImGui::Text(".");

		ImVec2 dotMax = ImGui::GetItemRectMax();	// 「.」の表示位置を基準に、Extensionの位置を設定
		float changeDrawPosX = 4.0f;				// 表示位置を変更するX値

		ImGui::SetCursorScreenPos(ImVec2(dotMax.x + changeDrawPosX, dotPos.y - 5.0f));

		// 拡張子 の入力欄の幅を指定
		//ImGui::SetNextItemWidth(280.0f - changeDrawPosX);

		// 拡張子名の入力
		if (ImGui::InputText(u8"拡張子", fileExtensionBuffer, 256))
		{
			createFileImGuiWindowData.fileExtension = fileExtensionBuffer;
		}
	}

	// ファイル名の入力
	if (ImGui::InputText(u8"名前", fileNameBuffer, 256))
	{
		createFileImGuiWindowData.fileName = fileNameBuffer;
	}

	// ファイル名が設定されているとき
	if (!createFileImGuiWindowData.fileName.empty())
	{
		// ファイルタイプがNONEでないとき、または、ファイルタイプがNONEかつファイルの拡張子が指定されていたら (文字数が「.」の一文字よりも多かったら)
		if (createFileImGuiWindowData.fileType != CLASS_FILE_TYPE::NONE ||
			(createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::NONE && createFileImGuiWindowData.fileExtension.size() > 1))
		{
			if (ImGui::Button(u8"追加"))
			{
				if (createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::NONE)
				{
					std::string filePath = createFileImGuiWindowData.filePath;	// ファイルパス
					filePath += "/";
					filePath += createFileImGuiWindowData.fileName;
					filePath += ".";
					filePath += createFileImGuiWindowData.fileExtension;
					
					// ユーザー指定の拡張子のファイル作成
					isCreateFile = FileSystemUtils::CreateEntryFile(filePath);
				}
				else if (createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::CLASS)
				{
					// Classファイルの作成
#ifdef VISUAL_STUDIO_ADD
					isCreateFile = VisualStudioProjectEditor::CreateClassFiles(createFileImGuiWindowData.filePath, createFileImGuiWindowData.fileName);
#else
					isCreateFile = CreateClassFiles(createFileImGuiWindowData.filePath, createFileImGuiWindowData.fileName);
#endif // VISUAL_STUDIO_ADD
				}
				else if (createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::CPP || createFileImGuiWindowData.fileType == CLASS_FILE_TYPE::H)
				{
					// .cppまたは.hファイルの作成
					isCreateFile = CreateCppFiles(createFileImGuiWindowData.filePath, createFileImGuiWindowData.fileName, createFileImGuiWindowData.fileType);
				}
				else
				{
					assert(false && "想定と異なるファイルタイプです。");
				}

				// 文字列をリセット //
				strcpy_s(fileExtensionBuffer, "");
				strcpy_s(fileNameBuffer, "");

				// ファイル名文字列のリセット
				createFileImGuiWindowData.fileName.clear();
			}
		}
	}

	if (ImGui::Button(u8"キャンセル"))
	{
		// 文字列をリセット //
		strcpy_s(fileExtensionBuffer, "");
		strcpy_s(fileNameBuffer, "");

		// ファイル関連のメンバ値のリセット
		createFileImGuiWindowData.ResetFileData();
		ImGui::End();
		return isCreateFile;	// Window表示をOFFにしたので、return
	}

	ImGui::End();

	return isCreateFile;
}

bool FileExplorer::CreateClassFiles(std::string _entryPath, std::string _className)
{
	//_ .hファイルの作成 _//

	std::string hInString = "#pragma once\n";	// .hファイルに入力する文字列
	hInString += "\n";
	hInString += "class " + _className + "\n";
	hInString += "{\n";
	hInString += "public:\n";
	hInString += "    ";
	hInString += _className + "();\n";
	hInString += "    ";
	hInString += "~" + _className + "();\n";
	hInString += "private:\n";
	hInString += "};\n";

	// .hファイルの作成
	bool isCreateFile = FileSystemUtils::CreateEntryFile(_entryPath + "/" + _className + ".h", hInString);	// ファイルを作成したかどうか

	//_ .cppファイルの作成 _//

	std::string cppInString = "#include ";	// .cppファイルに入力する文字列
	cppInString += "\"";
	cppInString += _className;
	cppInString += ".h";
	cppInString += "\"";

	// .cppファイルの作成
	isCreateFile &= (int)FileSystemUtils::CreateEntryFile(_entryPath + "/" + _className + ".cpp", cppInString);

	return isCreateFile;	// どちらのファイルも作成できたら
}

bool FileExplorer::CreateCppFiles(std::string _entryPath, std::string _cppFilesName, const CLASS_FILE_TYPE& _type)
{
	//_ .cppファイルの作成 _//

	if (_type == CLASS_FILE_TYPE::CPP)
	{
		std::string cppInString = "#include ";	// .cppファイルに入力する文字列
		cppInString += "\"";
		cppInString += _cppFilesName;
		cppInString += ".h";
		cppInString += "\"";
		std::string filePath = _entryPath;		// .cppファイルパス
		filePath += "/";
		filePath += _cppFilesName;
		filePath += ".cpp";

		// .cppファイルの作成
		return FileSystemUtils::CreateEntryFile(filePath, cppInString);
	}	
	//_ .hファイルの作成 _//
	else if (_type == CLASS_FILE_TYPE::H)
	{
		std::string hInString = "#pragma once\n";	// .hファイルに入力する文字列
		std::string filePath = _entryPath;			// .hファイルパス
		filePath += "/";
		filePath += _cppFilesName;
		filePath += ".h";

		// .hファイルの作成
		return FileSystemUtils::CreateEntryFile(filePath, hInString);
	}

	assert(false && ".cppまたは.hのファイル作成のファイル指定タイプが異なります。");

	return false;
}

//-------------------------------------------------------