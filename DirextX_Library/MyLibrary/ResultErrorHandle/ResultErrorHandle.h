#pragma once

class ResultErrorHandle
{
public:

    enum class ErrorCode
    {
        NotInitializationErrorCode = -1, //値の入れ忘れ
        OK = 0,               // エラーなし

        // ファイル関連
        FileNotFound = 100,       // ファイルが存在しない
        DirectoryNotFound = 101,  // フォルダが存在しない
        InvalidPath = 102,        // パスが不正
        AccessDenied = 103,       // アクセス権限がない

        // データ関連
        InvalidFormat = 200,      // 対応していない形式
        LoadFailed = 201,        // 読み込み失敗
        ParseFailed = 202,        // データ解析失敗
        InvalidData = 203,        // データが不正

        // GPU関連
        GPUResourceCreateFailed = 300,
        UploadFailed = 301,

        // その他
        Unknown = 1000,
    };

    ResultErrorHandle() {};
    ~ResultErrorHandle() {};

    bool ErrorCodeInit()const;

    bool IsSuccess()const;

    ErrorCode GetErrorCode()const { return errorCode; }
    void SetErrorCode(ErrorCode _code) { errorCode = _code; }

private:

    ErrorCode errorCode = ErrorCode::NotInitializationErrorCode;

};

