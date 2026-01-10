//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "pch.h"
#include "HardwareInfo.h"
#include "GraphicsCore.h"
#include <dxgi1_4.h>
#include <intrin.h>
#include <winternl.h>

namespace Benchmark
{
    namespace
    {
        GpuInfo s_GpuInfo;
        CpuInfo s_CpuInfo;
        uint64_t s_SystemRamMB = 0;
        std::string s_OSVersion;
        Microsoft::WRL::ComPtr<IDXGIAdapter3> s_Adapter;

        std::string WideToUtf8(const wchar_t* wide)
        {
            if (!wide || !*wide) return "";
            int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
            if (len <= 0) return "";
            std::string result(len - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide, -1, &result[0], len, nullptr, nullptr);
            return result;
        }

        std::string GetVendorName(uint32_t vendorId)
        {
            switch (vendorId)
            {
            case 0x10DE: return "NVIDIA";
            case 0x1002: return "AMD";
            case 0x8086: return "Intel";
            case 0x1414: return "Microsoft";
            default:     return "Unknown";
            }
        }

        std::string GetFeatureLevelString(D3D_FEATURE_LEVEL level)
        {
            switch (level)
            {
            case D3D_FEATURE_LEVEL_12_2: return "12_2";
            case D3D_FEATURE_LEVEL_12_1: return "12_1";
            case D3D_FEATURE_LEVEL_12_0: return "12_0";
            case D3D_FEATURE_LEVEL_11_1: return "11_1";
            case D3D_FEATURE_LEVEL_11_0: return "11_0";
            default: return "Unknown";
            }
        }

        std::string GetShaderModelString(ID3D12Device* device)
        {
            D3D12_FEATURE_DATA_SHADER_MODEL sm = { D3D_SHADER_MODEL_6_9 };
            if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &sm, sizeof(sm))))
            {
                switch (sm.HighestShaderModel)
                {
                case D3D_SHADER_MODEL_6_9: return "6.9";
                case D3D_SHADER_MODEL_6_8: return "6.8";
                case D3D_SHADER_MODEL_6_7: return "6.7";
                case D3D_SHADER_MODEL_6_6: return "6.6";
                default: return "6.x";
                }
            }
            return "Unknown";
        }

        void QueryGpuInfo()
        {
            if (!Graphics::g_Device) return;

            LUID luid = Graphics::g_Device->GetAdapterLuid();
            Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
            if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) return;

            Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter1;
            if (FAILED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&adapter1)))) return;

            adapter1.As(&s_Adapter);

            DXGI_ADAPTER_DESC1 desc;
            if (SUCCEEDED(adapter1->GetDesc1(&desc)))
            {
                s_GpuInfo.Name = WideToUtf8(desc.Description);
                s_GpuInfo.Vendor = GetVendorName(desc.VendorId);
                s_GpuInfo.VramMB = desc.DedicatedVideoMemory / (1024 * 1024);
            }

            s_GpuInfo.FeatureLevel = GetFeatureLevelString(Graphics::g_D3DFeatureLevel);
            s_GpuInfo.ShaderModel = GetShaderModelString(Graphics::g_Device);

            LARGE_INTEGER driverVersion;
            if (SUCCEEDED(adapter1->CheckInterfaceSupport(__uuidof(IDXGIDevice), &driverVersion)))
            {
                char ver[64];
                sprintf_s(ver, "%d.%d.%d.%d",
                    HIWORD(driverVersion.HighPart), LOWORD(driverVersion.HighPart),
                    HIWORD(driverVersion.LowPart), LOWORD(driverVersion.LowPart));
                s_GpuInfo.DriverVersion = ver;
            }
        }

        void QueryCpuInfo()
        {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            s_CpuInfo.Cores = sysInfo.dwNumberOfProcessors;
            s_CpuInfo.Threads = sysInfo.dwNumberOfProcessors;

            int cpuInfo[4] = { 0 };
            char brand[49] = { 0 };
            __cpuid(cpuInfo, 0x80000000);
            if ((unsigned)cpuInfo[0] >= 0x80000004)
            {
                __cpuid(reinterpret_cast<int*>(brand), 0x80000002);
                __cpuid(reinterpret_cast<int*>(brand + 16), 0x80000003);
                __cpuid(reinterpret_cast<int*>(brand + 32), 0x80000004);
                s_CpuInfo.Name = brand;
                size_t start = s_CpuInfo.Name.find_first_not_of(' ');
                if (start != std::string::npos)
                    s_CpuInfo.Name = s_CpuInfo.Name.substr(start);
            }

            HKEY hKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                DWORD mhz = 0, size = sizeof(DWORD);
                if (RegQueryValueExA(hKey, "~MHz", nullptr, nullptr,
                    reinterpret_cast<LPBYTE>(&mhz), &size) == ERROR_SUCCESS)
                    s_CpuInfo.FrequencyMHz = mhz;
                RegCloseKey(hKey);
            }
        }

        void QuerySystemRam()
        {
            MEMORYSTATUSEX memStatus = { sizeof(memStatus) };
            if (GlobalMemoryStatusEx(&memStatus))
                s_SystemRamMB = memStatus.ullTotalPhys / (1024 * 1024);
        }

        void QueryOSVersion()
        {
            OSVERSIONINFOEXW osvi = { sizeof(osvi) };
            typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
            if (HMODULE hMod = GetModuleHandleW(L"ntdll.dll"))
            {
                auto fn = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(hMod, "RtlGetVersion"));
                if (fn) fn(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi));
            }
            char ver[128];
            sprintf_s(ver, "Windows %lu.%lu.%lu",
                osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
            s_OSVersion = ver;
        }
    }

    namespace HardwareInfo
    {
        void Initialize()
        {
            QueryGpuInfo();
            QueryCpuInfo();
            QuerySystemRam();
            QueryOSVersion();
        }

        GpuInfo GetGpuInfo() { return s_GpuInfo; }
        CpuInfo GetCpuInfo() { return s_CpuInfo; }
        uint64_t GetSystemRamMB() { return s_SystemRamMB; }
        std::string GetOSVersion() { return s_OSVersion; }

        VramUsage GetVramUsage()
        {
            VramUsage usage;
            if (s_Adapter)
            {
                DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
                if (SUCCEEDED(s_Adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo)))
                {
                    usage.CommittedMB = memInfo.CurrentUsage / (1024 * 1024);
                    usage.BudgetMB = memInfo.Budget / (1024 * 1024);
                }
            }
            return usage;
        }
    }
}