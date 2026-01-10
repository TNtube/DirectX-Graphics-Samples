//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <string>
#include <cstdint>

namespace Benchmark
{
    struct GpuInfo
    {
        std::string Name;
        std::string Vendor;
        std::string DriverVersion;
        uint64_t VramMB = 0;
        std::string FeatureLevel;
        std::string ShaderModel;
    };

    struct CpuInfo
    {
        std::string Name;
        uint32_t Cores = 0;
        uint32_t Threads = 0;
        uint32_t FrequencyMHz = 0;
    };

    struct VramUsage
    {
        uint64_t CommittedMB = 0;
        uint64_t BudgetMB = 0;
    };

    namespace HardwareInfo
    {
        void Initialize();
        GpuInfo GetGpuInfo();
        CpuInfo GetCpuInfo();
        uint64_t GetSystemRamMB();
        std::string GetOSVersion();
        VramUsage GetVramUsage();
    }
}