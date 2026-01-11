//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "pch.h"
#include "MemoryTracker.h"
#include "HardwareInfo.h"
#include <chrono>

namespace Benchmark
{
    namespace
    {
        std::vector<MemorySample> s_Samples;
        uint64_t s_PeakCommittedMB = 0;
        std::chrono::steady_clock::time_point s_StartTime;
    }

    namespace MemoryTracker
    {
        void Initialize()
        {
            s_StartTime = std::chrono::steady_clock::now();
            s_Samples.clear();
            s_Samples.reserve(10000);
            s_PeakCommittedMB = 0;
        }

        void Sample()
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_StartTime);

            VramUsage usage = HardwareInfo::GetVramUsage();

            MemorySample sample;
            sample.TimestampMs = elapsed.count();
            sample.CommittedMB = usage.CommittedMB;
            sample.BudgetMB = usage.BudgetMB;

            s_Samples.push_back(sample);

            if (usage.CommittedMB > s_PeakCommittedMB)
                s_PeakCommittedMB = usage.CommittedMB;
        }

        void Reset()
        {
            Initialize();
        }

        const std::vector<MemorySample>& GetSamples()
        {
            return s_Samples;
        }

        uint64_t GetPeakCommittedMB()
        {
            return s_PeakCommittedMB;
        }

        uint64_t GetAverageCommittedMB()
        {
            if (s_Samples.empty())
                return 0;

            uint64_t sum = 0;
            for (const auto& s : s_Samples)
                sum += s.CommittedMB;

            return sum / s_Samples.size();
        }
    }
}