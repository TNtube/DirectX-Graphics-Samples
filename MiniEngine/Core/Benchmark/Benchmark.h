//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <string>
#include <cstdint>

namespace Benchmark
{
    enum class State
    {
        Idle,
        Warmup,
        Measuring,
        Complete
    };

    void Initialize();
    void Shutdown();

    void StartRun(const char* sceneName, const char* precisionMode);
    void EndRun();
    void Reset();

    State GetState();
    bool IsRunning();
    bool IsMeasuring();

    uint32_t RecordFrame(float cpuTimeMs, float gpuTimeMs);
    uint32_t GetCurrentFrame();
    uint32_t GetWarmupFrames();
    uint32_t GetMeasuredFrames();

    void SetWarmupFrames(uint32_t count);
    void SetMeasuredFrames(uint32_t count);
    void SetMemorySampleInterval(uint32_t frames);

    void ExportToJson(const char* outputPath);
}