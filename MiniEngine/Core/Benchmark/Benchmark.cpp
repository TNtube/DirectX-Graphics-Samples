//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "pch.h"
#include "Benchmark.h"
#include "Statistics.h"
#include "HardwareInfo.h"
#include "MemoryTracker.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

namespace Benchmark
{
    namespace
    {
        State s_State = State::Idle;
        std::string s_SceneName;
        std::string s_PrecisionMode;

        uint32_t s_WarmupFrames = 120;
        uint32_t s_MeasuredFrames = 1000;
        uint32_t s_MemorySampleInterval = 60;
        uint32_t s_CurrentFrame = 0;

        std::vector<float> s_CpuTimes;
        std::vector<float> s_GpuTimes;
        std::vector<float> s_FrameTimes;

        uint64_t s_StartVramMB = 0;

        std::string GetTimestamp()
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_s(&tm, &time);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
            return oss.str();
        }
    }

    void Initialize()
    {
        s_State = State::Idle;
        s_CpuTimes.reserve(s_MeasuredFrames);
        s_GpuTimes.reserve(s_MeasuredFrames);
        s_FrameTimes.reserve(s_MeasuredFrames);
    }

    void Shutdown()
    {
        s_CpuTimes.clear();
        s_GpuTimes.clear();
        s_FrameTimes.clear();
    }

    void StartRun(const char* sceneName, const char* precisionMode)
    {
        s_SceneName = sceneName ? sceneName : "unknown";
        s_PrecisionMode = precisionMode ? precisionMode : "fp32_baseline";
        s_CurrentFrame = 0;
        s_State = State::Warmup;

        s_CpuTimes.clear();
        s_GpuTimes.clear();
        s_FrameTimes.clear();

        MemoryTracker::Reset();
        VramUsage usage = HardwareInfo::GetVramUsage();
        s_StartVramMB = usage.CommittedMB;
        MemoryTracker::Sample();
    }

    void EndRun()
    {
        if (s_State == State::Measuring)
        {
            MemoryTracker::Sample();
            s_State = State::Complete;
        }
    }

    void Reset()
    {
        s_State = State::Idle;
        s_CurrentFrame = 0;
        s_CpuTimes.clear();
        s_GpuTimes.clear();
        s_FrameTimes.clear();
        MemoryTracker::Reset();
    }

    State GetState() { return s_State; }
    bool IsRunning() { return s_State == State::Warmup || s_State == State::Measuring; }
    bool IsMeasuring() { return s_State == State::Measuring; }
    uint32_t GetCurrentFrame() { return s_CurrentFrame; }
    uint32_t GetWarmupFrames() { return s_WarmupFrames; }
    uint32_t GetMeasuredFrames() { return s_MeasuredFrames; }

    void SetWarmupFrames(uint32_t count) { s_WarmupFrames = count; }
    void SetMeasuredFrames(uint32_t count) { s_MeasuredFrames = count; }
    void SetMemorySampleInterval(uint32_t frames) { s_MemorySampleInterval = frames; }

    void RecordFrame(float cpuTimeMs, float gpuTimeMs)
    {
        if (s_State == State::Idle || s_State == State::Complete)
            return;

        s_CurrentFrame++;

        if (s_State == State::Warmup)
        {
            if (s_CurrentFrame >= s_WarmupFrames)
            {
                s_State = State::Measuring;
                s_CurrentFrame = 0;
                MemoryTracker::Sample();
            }
            return;
        }

        float frameTime = cpuTimeMs + gpuTimeMs;
        s_CpuTimes.push_back(cpuTimeMs);
        s_GpuTimes.push_back(gpuTimeMs);
        s_FrameTimes.push_back(frameTime);

        if (s_CurrentFrame % s_MemorySampleInterval == 0)
            MemoryTracker::Sample();

        if (s_CurrentFrame >= s_MeasuredFrames)
            EndRun();
    }

    void ExportToJson(const char* outputPath)
    {
        if (s_State != State::Complete)
            return;

        GpuInfo gpu = HardwareInfo::GetGpuInfo();
        CpuInfo cpu = HardwareInfo::GetCpuInfo();

        Statistics cpuStats = Statistics::Compute(s_CpuTimes);
        Statistics gpuStats = Statistics::Compute(s_GpuTimes);
        Statistics frameStats = Statistics::Compute(s_FrameTimes);

        std::vector<float> fpsValues;
        fpsValues.reserve(s_FrameTimes.size());
        for (float ft : s_FrameTimes)
            fpsValues.push_back(ft > 0.0f ? 1000.0f / ft : 0.0f);
        Statistics fpsStats = Statistics::Compute(fpsValues);

        json output;
        output["metadata"] = {
            {"timestamp", GetTimestamp()},
            {"precision_mode", s_PrecisionMode},
            {"build_config", "Release"},
            {"hardware", {
                {"gpu", {
                    {"name", gpu.Name},
                    {"vendor", gpu.Vendor},
                    {"driver_version", gpu.DriverVersion},
                    {"vram_mb", gpu.VramMB},
                    {"feature_level", gpu.FeatureLevel},
                    {"shader_model", gpu.ShaderModel}
                }},
                {"cpu", {
                    {"name", cpu.Name},
                    {"cores", cpu.Cores},
                    {"threads", cpu.Threads},
                    {"frequency_mhz", cpu.FrequencyMHz}
                }},
                {"ram_gb", HardwareInfo::GetSystemRamMB() / 1024.0},
                {"os", HardwareInfo::GetOSVersion()}
            }},
            {"scene", {{"name", s_SceneName}}},
            {"test_params", {
                {"warmup_frames", s_WarmupFrames},
                {"measured_frames", s_MeasuredFrames}
            }}
        };

        output["performance"] = {
            {"frame_time_ms", frameStats.ToJson()},
            {"fps", fpsStats.ToJson()},
            {"cpu_time_ms", {{"total", cpuStats.ToJson()}}},
            {"gpu_time_ms", {{"total", gpuStats.ToJson()}}}
        };

        json perFrameData = json::array();
        for (size_t i = 0; i < s_FrameTimes.size(); ++i)
        {
            perFrameData.push_back({
                {"frame_index", i},
                {"frame_time_ms", s_FrameTimes[i]},
                {"cpu_total_ms", s_CpuTimes[i]},
                {"gpu_total_ms", s_GpuTimes[i]}
            });
        }
        output["performance"]["per_frame_data"] = perFrameData;

        VramUsage endUsage = HardwareInfo::GetVramUsage();
        output["memory"] = {
            {"vram_mb", {
                {"committed_start", s_StartVramMB},
                {"committed_end", endUsage.CommittedMB},
                {"committed_peak", MemoryTracker::GetPeakCommittedMB()},
                {"budget", endUsage.BudgetMB}
            }}
        };

        output["quality"] = json::object();

        std::ofstream file(outputPath);
        if (file.is_open())
        {
            file << output.dump(2);
            file.close();
        }
    }
}