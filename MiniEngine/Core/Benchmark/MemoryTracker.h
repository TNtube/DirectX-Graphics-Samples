//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <vector>
#include <cstdint>

namespace Benchmark
{
    struct MemorySample
    {
        uint64_t TimestampMs;
        uint64_t CommittedMB;
        uint64_t BudgetMB;
    };

    namespace MemoryTracker
    {
        void Initialize();
        void Sample();
        void Reset();

        const std::vector<MemorySample>& GetSamples();
        uint64_t GetPeakCommittedMB();
        uint64_t GetAverageCommittedMB();
    }
}