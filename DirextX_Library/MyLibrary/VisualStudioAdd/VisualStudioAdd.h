#pragma once
#include <filesystem>
#include <string>

//#define TEST

class VisualStudioProjectEditor
{
public:

    VisualStudioProjectEditor() = delete;
    // クラスの .h / .cpp をプロジェクトに登録し、
    // Generated フィルターを作成する。
    static bool AddClassToProject(const std::filesystem::path& projectPath,const std::filesystem::path& entryPath,const std::string& className);

    static bool CreateClassFiles(std::string _entryPath, std::string _className);

    static bool ProjectFilePath(const std::filesystem::path& projectPath);

private:

    static std::filesystem::path projectFilePath;
};

