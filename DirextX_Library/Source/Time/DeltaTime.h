#pragma once
#include <time.h>
#include <stdio.h>
#include <chrono>
#include "../SingleTon/SingletonBase.h"

class DeltaTime : public SingletonBase<DeltaTime>
{
public:

	float GetFPS()const { return currentFPS; }
	float GetDeltaTime()const { return deltaTime; }
	float GetDeltaTimeMulTimeScale()const { return deltaTime * timeScale; }
	void SetTimeScale(float _timeScale) { timeScale = _timeScale; }
	// 戻り値の型が bool になっていたため、void（または適切な実装）に合わせて修正
	void SetFrameRateLimitEnabled(bool _limitFps) { isFpsLimit = _limitFps; }

private:

	DeltaTime();
	~DeltaTime();

	void Update();

	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point currentTime;
	std::chrono::duration<float> subTime;
	float deltaTime;
	std::chrono::nanoseconds frameDuration;
	const std::chrono::nanoseconds targetFrameTime;

	int frameCount;
	float fpsTimer;
	float currentFPS;

	bool isFpsLimit;

	float timeScale;

	static const int BUF_SIZE = 30;
	float timeBuf[BUF_SIZE];
	int readP;
	int writeP;

	friend class SingletonBase<DeltaTime>;
	friend class Main;
};