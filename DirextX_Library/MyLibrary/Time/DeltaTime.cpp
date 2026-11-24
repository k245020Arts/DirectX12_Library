#include "DeltaTime.h"
#include <thread>
#include <iostream>

namespace {
    const int FPS = 120;
    //デバックでストップした時にデルタタイムがデカくなりすぎないように制御
    const float maxDeltaTime = 0.5f;
}

void DeltaTime::TimeMeasurementStart()
{
    timerMeasurementStart = std::chrono::steady_clock::now();
}

void DeltaTime::TimeMeasurementFinish(const std::string& _message)
{
    timerMeasurementFinish = std::chrono::steady_clock::now();

    timeTaken = timerMeasurementFinish - timerMeasurementStart;

    // ミリ秒に変換
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeTaken).count();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeTaken).count();

    std::string MSTitleMessage = "Time Taken:";
    std::string MSEndMessage = "%lld ms\n";

    std::string MSmessage = MSTitleMessage + _message + MSEndMessage;

    std::string SEndMessage = "%.3f 秒\n";

    std::string SMessage = MSTitleMessage + _message + SEndMessage;

    printf(MSmessage.c_str(), static_cast<long long>(milliseconds));

    printf(SMessage.c_str(), static_cast<float>(milliseconds / 1000.0f));
}

DeltaTime::DeltaTime() : targetFrameTime(std::chrono::microseconds(1000000 / FPS))
{
    startTime = std::chrono::high_resolution_clock::now();
    currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = 0.0f;
    frameDuration = std::chrono::nanoseconds();

    subTime = std::chrono::duration<float>();

    frameCount = 0;
    fpsTimer = 0.0f;
    currentFPS = 0.0f;

    isFpsLimit = true;
    timeScale = 1.0f;

    // バッファ用変数の初期化
    readP = 0;
    writeP = 0;
    for (int i = 0; i < BUF_SIZE; ++i) {
        timeBuf[i] = 0.0f;
    }
}

DeltaTime::~DeltaTime()
{

}

void DeltaTime::Update()
{
    // 固定FPSモードがオンならこのループの中に入る
    while (isFpsLimit) {
        auto nowTime = std::chrono::high_resolution_clock::now();
        auto elapsed = nowTime - currentTime; // 前回確定時からの経過時間

        if (elapsed >= targetFrameTime) {
            break; // 目標時間に達したらループを抜ける
        }

        if (targetFrameTime - elapsed > std::chrono::milliseconds(1)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(0));
        }
    }

    // 時間の計測（前フレームとの正確な差分を秒単位で取得）
    auto last = currentTime;
    currentTime = std::chrono::high_resolution_clock::now();

    subTime = currentTime - last;
    float dt = subTime.count();

    timeBuf[writeP] = dt;
    writeP = (writeP + 1) % BUF_SIZE;
    if (writeP == readP) {
        readP = (readP + 1) % BUF_SIZE;
    }

    float sum = 0;
    int num = 0;
    for (int i = readP; i != writeP; i = (i + 1) % BUF_SIZE) {
        sum += timeBuf[i % BUF_SIZE];
        num++;
    }

    if (num > BUF_SIZE / 2) {
        float ave = sum / num;
        if (dt >= ave * 2) {
            deltaTime = ave * 2;
        }
        else {
            deltaTime = dt;
        }
    }
    else {
        deltaTime = dt;
    }

    // セーフティ（maxDeltaTime）も最終防衛線として適用
    if (deltaTime > maxDeltaTime) {
        deltaTime = maxDeltaTime;
    }


    if (num > 0 && sum > 0.0f) {
        currentFPS = static_cast<float>(num) / sum;
    }
    else {
        currentFPS = static_cast<float>(FPS);
    }
}
