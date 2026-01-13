//
// FrameCapture.h - Frame capture utility for image quality comparison
//

#pragma once

#include <cstdint>

namespace Benchmark
{
    void InitializeFrameCapture();
    void ShutdownFrameCapture();

    // Capture the current frame to a PNG file
    bool CaptureFrame(const char* outputPath);

    void SetCaptureInterval(uint32_t frameInterval);
    uint32_t GetCaptureInterval();

    void SetCaptureOutputDir(const char* dir);
    const char* GetCaptureOutputDir();

    bool IsCaptureEnabled();
    void SetCaptureEnabled(bool enabled);

    void UpdateFrameCapture(uint32_t frameNumber);
}