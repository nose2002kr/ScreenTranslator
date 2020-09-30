#pragma once

#include "pch.h"
#include "common_util.h"
#include "SimpleCapture.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::Foundation;
    using namespace Windows::System;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Graphics::DirectX::Direct3D11;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Storage::Streams;
}

class CaptureSnapshot
{
public:
    static CaptureSnapshot& inst() {
        static CaptureSnapshot* instance = new CaptureSnapshot();
        return *instance;
    }
    Image* takeImage(HWND hWnd);
    winrt::IAsyncOperation<winrt::IDirect3DSurface>
        takeAsync(HWND hWnd);

private:
    CaptureSnapshot();

    winrt::IDirect3DDevice m_device{ nullptr };
    void startCaptureFromItem(winrt::GraphicsCaptureItem item);
    void stopCapture();

    winrt::Compositor m_compositor{ nullptr };
    winrt::CompositionSurfaceBrush m_brush{ nullptr };
    std::unique_ptr<SimpleCapture> m_capture{ nullptr };
};