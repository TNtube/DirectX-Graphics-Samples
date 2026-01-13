//
// FrameCapture.cpp - Frame capture implementation
//

#include "pch.h"
#include "FrameCapture.h"
#include "BufferManager.h"
#include "CommandContext.h"
#include "ReadbackBuffer.h"
#include "GraphicsCore.h"
#include "Display.h"
#include <DirectXTex.h>
#include <filesystem>
#include <wincodec.h>

using namespace Graphics;

namespace Benchmark
{
    static ReadbackBuffer s_ReadbackBuffer;
    static bool s_Initialized = false;
    static bool s_CaptureEnabled = false;
    static uint32_t s_CaptureInterval = 0;
    static std::string s_OutputDir = "captures";
    static uint32_t s_CaptureCount = 0;

    void InitializeFrameCapture()
    {
        if (s_Initialized)
            return;

        uint32_t width = g_SceneColorBuffer.GetWidth();
        uint32_t height = g_SceneColorBuffer.GetHeight();
        uint32_t rowPitch = width * 4; // RGBA8

        s_ReadbackBuffer.Create(L"Frame Capture Readback", height * rowPitch, 1);
        s_Initialized = true;
        s_CaptureCount = 0;
    }

    void ShutdownFrameCapture()
    {
        if (!s_Initialized)
            return;

        s_ReadbackBuffer.Destroy();
        s_Initialized = false;
    }

    bool CaptureFrame(const char* outputPath)
    {
        if (!s_Initialized)
            InitializeFrameCapture();

        uint32_t width = g_SceneColorBuffer.GetWidth();
        uint32_t height = g_SceneColorBuffer.GetHeight();
        uint32_t rowPitch = width * 4;

        // Ensure readback buffer is large enough
        if (s_ReadbackBuffer.GetBufferSize() < height * rowPitch)
        {
            s_ReadbackBuffer.Destroy();
            s_ReadbackBuffer.Create(L"Frame Capture Readback", height * rowPitch, 1);
        }

        GraphicsContext& context = GraphicsContext::Begin(L"Frame Capture");

        // Transition scene color to copy source
        context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

        // Get texture footprint
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        D3D12_RESOURCE_DESC srcDesc = g_SceneColorBuffer.GetResource()->GetDesc();
        g_Device->GetCopyableFootprints(&srcDesc, 0, 1, 0, &footprint, nullptr, nullptr, nullptr);

        // Copy texture to readback buffer
        CD3DX12_TEXTURE_COPY_LOCATION srcLoc(g_SceneColorBuffer.GetResource(), 0);
        CD3DX12_TEXTURE_COPY_LOCATION dstLoc(s_ReadbackBuffer.GetResource(), footprint);
        context.GetCommandList()->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

        // Transition back
        context.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

        context.Finish(true); // Wait for completion

        // Map and read the data
        void* mappedData = s_ReadbackBuffer.Map();
        if (!mappedData)
        {
            Utility::Printf("FrameCapture: Failed to map readback buffer\n");
            return false;
        }

        DirectX::Image image = {};
        image.width = width;
        image.height = height;
        image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        image.rowPitch = footprint.Footprint.RowPitch;
        image.slicePitch = image.rowPitch * height;
        image.pixels = static_cast<uint8_t*>(mappedData);

        // Convert from R11G11B10 to RGBA8 if needed
        DirectX::ScratchImage converted;
        HRESULT hr = S_OK;

        if (srcDesc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
        {
            // Source is HDR format, need conversion
            DirectX::ScratchImage srcImage;
            srcImage.Initialize2D(srcDesc.Format, width, height, 1, 1);
            memcpy(srcImage.GetPixels(), mappedData, srcImage.GetPixelsSize());

            hr = DirectX::Convert(*srcImage.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM,
                DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, converted);

            if (FAILED(hr))
            {
                s_ReadbackBuffer.Unmap();
                Utility::Printf("FrameCapture: Failed to convert image format\n");
                return false;
            }
            image = *converted.GetImage(0, 0, 0);
        }

        std::filesystem::path outPath(outputPath);
        if (outPath.has_parent_path())
            std::filesystem::create_directories(outPath.parent_path());

        std::wstring wpath = outPath.wstring() + L".png";
        hr = DirectX::SaveToWICFile(image, DirectX::WIC_FLAGS_NONE,
            GUID_ContainerFormatPng, wpath.c_str());

        s_ReadbackBuffer.Unmap();

        if (FAILED(hr))
        {
            Utility::Printf("FrameCapture: Failed to save PNG: %s\n", outputPath);
            return false;
        }

        Utility::Printf("FrameCapture: Saved %s.png\n", outputPath);
        return true;
    }

    void SetCaptureInterval(uint32_t frameInterval)
    {
        s_CaptureInterval = frameInterval;
        s_CaptureEnabled = (frameInterval > 0);
    }

    uint32_t GetCaptureInterval()
    {
        return s_CaptureInterval;
    }

    void SetCaptureOutputDir(const char* dir)
    {
        s_OutputDir = dir ? dir : "captures";
    }

    const char* GetCaptureOutputDir()
    {
        return s_OutputDir.c_str();
    }

    bool IsCaptureEnabled()
    {
        return s_CaptureEnabled;
    }

    void SetCaptureEnabled(bool enabled)
    {
        s_CaptureEnabled = enabled;
    }

    void UpdateFrameCapture(uint32_t frameNumber)
    {
        if (!s_CaptureEnabled || s_CaptureInterval == 0)
            return;

        if (frameNumber % s_CaptureInterval == 0)
        {
            char filename[256];
            snprintf(filename, sizeof(filename), "%s/frame_%04u_capture_%04u", s_OutputDir.c_str(), frameNumber, s_CaptureCount++);
            CaptureFrame(filename);
        }
    }
}