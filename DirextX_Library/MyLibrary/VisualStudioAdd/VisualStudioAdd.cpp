#include "VisualStudioAdd.h"
#include "tinyxml2.h"

#include <Windows.h>
#include <objbase.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>
#include "../../KLibrary/File/FileExplorer/FileExplorer.h"
#include <algorithm>

namespace fs = std::filesystem;

std::filesystem::path VisualStudioProjectEditor::projectFilePath = std::filesystem::path();

namespace
{
    // ファイルをバイナリとして読み込む。
    bool ReadFile(const fs::path& path, std::string& out)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        out.assign(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());

        return !file.bad();
    }

    // ファイルをバイナリとして書き込む。
    bool WriteFile(const fs::path& path, const std::string& data)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file)
            return false;

        file.write(data.data(),
            static_cast<std::streamsize>(data.size()));

        file.flush();
        return file.good();
    }

    // tinyxml2::XMLDocument の内容を文字列化する。
    std::string ToXmlString(tinyxml2::XMLDocument& doc)
    {
        tinyxml2::XMLPrinter printer;
        doc.Print(&printer);
        return std::string(printer.CStr(), printer.CStrSize() - 1);
    }

    // 既存の ItemGroup を探す。なければ作成する。
    tinyxml2::XMLElement* FindOrCreateItemGroup(
        tinyxml2::XMLDocument& doc,
        tinyxml2::XMLElement* root,
        const char* itemName,
        const char* label = nullptr)
    {
        for (tinyxml2::XMLElement* group = root->FirstChildElement("ItemGroup");
            group;
            group = group->NextSiblingElement("ItemGroup"))
        {
            if (label)
            {
                const char* groupLabel = group->Attribute("Label");
                if (groupLabel && std::string(groupLabel) == label)
                    return group;
            }
            else if (group->FirstChildElement(itemName))
            {
                return group;
            }
        }

        tinyxml2::XMLElement* group = doc.NewElement("ItemGroup");
        if (label)
            group->SetAttribute("Label", label);

        root->InsertEndChild(group);
        return group;
    }

    // 同じ Include の要素がすでにあるか調べる。
    bool HasInclude(
        tinyxml2::XMLElement* root,
        const char* groupName,
        const std::string& include)
    {
        for (tinyxml2::XMLElement* group = root->FirstChildElement("ItemGroup");
            group;
            group = group->NextSiblingElement("ItemGroup"))
        {
            for (tinyxml2::XMLElement* item = group->FirstChildElement(groupName);
                item;
                item = item->NextSiblingElement(groupName))
            {
                const char* value = item->Attribute("Include");
                if (value && include == value)
                    return true;
            }
        }

        return false;
    }

    // vcxproj にソース・ヘッダーファイルを登録する。
    void AddProjectFile(
        tinyxml2::XMLDocument& doc,
        tinyxml2::XMLElement* root,
        const char* elementName,
        const std::string& include)
    {
        if (HasInclude(root, elementName, include))
            return;

        tinyxml2::XMLElement* group =
            FindOrCreateItemGroup(doc, root, elementName);

        tinyxml2::XMLElement* item = doc.NewElement(elementName);
        item->SetAttribute("Include", include.c_str());
        group->InsertEndChild(item);
    }

    // Windows API でフィルター用 GUID を作成する。
    bool CreateGuidString(std::string& out)
    {
        GUID guid{};
        if (FAILED(CoCreateGuid(&guid)))
            return false;

        wchar_t buffer[64]{};
        if (StringFromGUID2(guid, buffer, 64) == 0)
            return false;

        // GUID の文字は ASCII の範囲なので、そのまま変換できる。
        out.clear();
        for (const wchar_t* p = buffer; *p; ++p)
            out.push_back(static_cast<char>(*p));

        return true;
    }

    // 指定した名前のフィルターが存在するか調べる。
    bool HasFilter(tinyxml2::XMLElement* root, const std::string& filterName)
    {
        for (tinyxml2::XMLElement* group = root->FirstChildElement("ItemGroup");
            group;
            group = group->NextSiblingElement("ItemGroup"))
        {
            for (tinyxml2::XMLElement* filter = group->FirstChildElement("Filter");
                filter;
                filter = filter->NextSiblingElement("Filter"))
            {
                const char* name = filter->Attribute("Include");
                if (name && filterName == name)
                    return true;
            }
        }

        return false;
    }

    // フィルターがなければ作成する。
    bool AddFilterIfNeeded(
        tinyxml2::XMLDocument& doc,
        tinyxml2::XMLElement* root,
        const std::string& filterName)
    {
        if (HasFilter(root, filterName))
            return true;

        std::string guid;
        if (!CreateGuidString(guid))
            return false;

        tinyxml2::XMLElement* group =
            FindOrCreateItemGroup(doc, root, "Filter", "Filters");

        tinyxml2::XMLElement* filter = doc.NewElement("Filter");
        filter->SetAttribute("Include", filterName.c_str());

        tinyxml2::XMLElement* guidElement = doc.NewElement("UniqueIdentifier");
        guidElement->SetText(guid.c_str());

        filter->InsertEndChild(guidElement);
        group->InsertEndChild(filter);

        return true;
    }

    // .vcxproj.filters にファイルを登録する。
    void AddFilterFile(
        tinyxml2::XMLDocument& doc,
        tinyxml2::XMLElement* root,
        const char* elementName,
        const std::string& include,
        const std::string& filterName)
    {
        tinyxml2::XMLElement* group =
            FindOrCreateItemGroup(doc, root, elementName);

        // 同じファイルの登録があれば、Filter の値を更新する。
        for (tinyxml2::XMLElement* item = group->FirstChildElement(elementName);
            item;
            item = item->NextSiblingElement(elementName))
        {
            const char* value = item->Attribute("Include");
            if (value && include == value)
            {
                tinyxml2::XMLElement* filter = item->FirstChildElement("Filter");
                if (!filter)
                {
                    filter = doc.NewElement("Filter");
                    item->InsertEndChild(filter);
                }

                filter->SetText(filterName.c_str());
                return;
            }
        }

        tinyxml2::XMLElement* item = doc.NewElement(elementName);
        item->SetAttribute("Include", include.c_str());

        tinyxml2::XMLElement* filter = doc.NewElement("Filter");
        filter->SetText(filterName.c_str());

        item->InsertEndChild(filter);
        group->InsertEndChild(item);
    }

    bool CopyBackup(
        const fs::path& original,
        const fs::path& backup)
    {
        std::error_code ec;
        fs::copy_file(
            original,
            backup,
            fs::copy_options::none,
            ec);

        return !ec;
    }

    // 一時ファイルを置き換える。Windows で既存ファイルを置換する。
    bool ReplaceFile(
        const fs::path& tempPath,
        const fs::path& destination)
    {
        return MoveFileExW(
            tempPath.c_str(),
            destination.c_str(),
            MOVEFILE_REPLACE_EXISTING |
            MOVEFILE_WRITE_THROUGH) != 0;
    }

    bool SaveToTemp(
        const fs::path& tempPath,
        const std::string& contents)
    {
        return WriteFile(tempPath, contents);
    }

    // 元データを一時ファイル経由で復元する。
    bool RestoreFile(
        const fs::path& destination,
        const std::string& originalContents)
    {
        fs::path tempPath = destination;
        tempPath += L".rollback.tmp";

        if (!WriteFile(tempPath, originalContents))
            return false;

        if (!ReplaceFile(tempPath, destination))
        {
            std::error_code ec;
            fs::remove(tempPath, ec);
            return false;
        }

        return true;
    }

    // バックアップファイルの保存先とファイル名を生成する
    fs::path MakeBackupPath(
        const fs::path& backupDirectory,
        const fs::path& original)
    {
        const auto now =
            std::chrono::system_clock::now().time_since_epoch();

        const auto ticks =
            std::chrono::duration_cast<std::chrono::milliseconds>(now)
            .count();

        const std::wstring backupFileName =
            original.filename().wstring() +
            L"." + std::to_wstring(ticks) + L".bak";

        return backupDirectory / backupFileName;
    }

}

bool VisualStudioProjectEditor::AddClassToProject(const fs::path& projectPath,const fs::path& entryPath,const std::string& className)
{
    std::error_code ec;
    const fs::path project = fs::absolute(projectPath, ec);
    if (ec)
        return false;

    const fs::path filtersPath =
        project.parent_path() /
        (project.stem().wstring() + L".vcxproj.filters");

    const fs::path directory = fs::absolute(entryPath, ec);
    if (ec)
        return false;

    const fs::path headerPath = directory / (className + ".h");
    const fs::path sourcePath = directory / (className + ".cpp");

    // 登録対象のファイルが実際に存在することを確認する。
    if (!fs::is_regular_file(headerPath) ||
        !fs::is_regular_file(sourcePath))
    {
        return false;
    }

    // プロジェクトのディレクトリからの相対パスにする。
    const std::string headerInclude =
        fs::relative(headerPath, project.parent_path(), ec)
        .generic_string();
    if (ec)
        return false;

    const std::string sourceInclude =
        fs::relative(sourcePath, project.parent_path(), ec)
        .generic_string();
    if (ec)
        return false;

    std::string originalProject;
    std::string originalFilters;

    if (!ReadFile(project, originalProject) ||
        !ReadFile(filtersPath, originalFilters))
    {
        return false;
    }

    // バックアップ用フォルダを取得
    const fs::path backupDirectory =
        project.parent_path() / L"ProjectBackups";

    // フォルダがなければ作成する。
    // すでに存在する場合は、そのまま使用する。
    fs::create_directories(backupDirectory, ec);

    if (ec)
        return false;

    // バックアップファイルのパスを生成
    const fs::path projectBackup =
        MakeBackupPath(backupDirectory, project);

    const fs::path filtersBackup =
        MakeBackupPath(backupDirectory, filtersPath);

    // .vcxproj をバックアップ
    if (!CopyBackup(project, projectBackup))
        return false;

    // .vcxproj.filters をバックアップ
    if (!CopyBackup(filtersPath, filtersBackup))
    {
        fs::remove(projectBackup, ec);
        return false;
    }

    tinyxml2::XMLDocument projectDoc;
    tinyxml2::XMLDocument filtersDoc;

    if (projectDoc.Parse(
        originalProject.data(), originalProject.size()) !=
        tinyxml2::XML_SUCCESS ||
        filtersDoc.Parse(
            originalFilters.data(), originalFilters.size()) !=
        tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    tinyxml2::XMLElement* projectRoot = projectDoc.FirstChildElement("Project");
    tinyxml2::XMLElement* filtersRoot = filtersDoc.FirstChildElement("Project");

    if (!projectRoot || !filtersRoot)
        return false;

    // vcxproj にファイルを登録する。
    AddProjectFile(
        projectDoc, projectRoot, "ClInclude", headerInclude);

    AddProjectFile(
        projectDoc, projectRoot, "ClCompile", sourceInclude);

    // プロジェクトフォルダからの相対パスをフィルター名にする
    const fs::path relativeDirectory =
        fs::relative(directory, project.parent_path(), ec);

    if (ec)
        return false;

    // Visual Studio のフィルター階層にはバックスラッシュを使う
    std::string filterName = relativeDirectory.string();

    std::replace(filterName.begin(), filterName.end(), '/', '\\');

    if (!AddFilterIfNeeded(filtersDoc, filtersRoot, filterName))
        return false;

    AddFilterFile(
        filtersDoc, filtersRoot, "ClInclude",
        headerInclude, filterName);

    AddFilterFile(
        filtersDoc, filtersRoot, "ClCompile",
        sourceInclude, filterName);

    const std::string newProjectXml = ToXmlString(projectDoc);
    const std::string newFiltersXml = ToXmlString(filtersDoc);

    fs::path projectTemp = project;
    projectTemp += L".tmp";

    fs::path filtersTemp = filtersPath;
    filtersTemp += L".tmp";

    // まず両方の一時ファイルを書き出す。
    if (!SaveToTemp(projectTemp, newProjectXml) ||
        !SaveToTemp(filtersTemp, newFiltersXml))
    {
        fs::remove(projectTemp, ec);
        fs::remove(filtersTemp, ec);
        return false;
    }

    // 1つ目に成功して2つ目に失敗した場合も、元に戻す。
    if (!ReplaceFile(projectTemp, project))
    {
        fs::remove(projectTemp, ec);
        fs::remove(filtersTemp, ec);
        return false;
    }

    if (!ReplaceFile(filtersTemp, filtersPath))
    {
        fs::remove(filtersTemp, ec);

        const bool restored =
            RestoreFile(project, originalProject) &&
            RestoreFile(filtersPath, originalFilters);

        if (!restored)
        {
            // バックアップは復旧用に残す。
            return false;
        }

        return false;
    }

    // 成功時もバックアップは残す。
    return true;
}

bool VisualStudioProjectEditor::CreateClassFiles(std::string _entryPath,std::string _className)
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

    // ディレクトリのパスを取得
    fs::path directory = fs::u8path(_entryPath);

    // 3. 生成済みファイルを vcxproj と filters に登録する。
    if (!VisualStudioProjectEditor::AddClassToProject(
        projectFilePath, directory, _className))
    {
        std::error_code ec;
        return false;
    }

    return true;
}

bool VisualStudioProjectEditor::ProjectFilePath(const std::filesystem::path& projectPath)
{
    projectFilePath = std::filesystem::absolute(projectPath);
    return true;
}