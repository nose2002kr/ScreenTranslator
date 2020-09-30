#pragma once
#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>

namespace util
{
    inline auto CreateCaptureItemForWindow(HWND hwnd)
    {
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };
        try {
            auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
            winrt::check_hresult(interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item)));
        } catch (winrt::hresult_error const& ex) {
            if (ex.code() == 0x80040154) { // (0x80040154: Class not registered)
                MessageBox(nullptr, 
                    "Couldn't load COM Object.\n\
Your current system may not be the latest Windows 10.\n\
Try again after updating Windows.", "Error", MB_OK | MB_ICONERROR);
                exit(-1);
            }
        }
        return item;
    }

    inline auto CreateCaptureItemForMonitor(HMONITOR hmon)
    {
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };
        try {
            auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
            winrt::check_hresult(interop_factory->CreateForMonitor(hmon, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item)));
        } catch (winrt::hresult_error const& ex) {
            if (ex.code() == 0x80040154) { // (0x80040154: Class not registered)
                MessageBox(nullptr,
                    "Couldn't load COM Object.\n\
Your current system may not be the latest Windows 10.\n\
Try again after updating Windows.", "Error", MB_OK | MB_ICONERROR);
                exit(-1);
            }
        }
        return item;
    }
}
