#include "pch.h"
#include "CaptureSnapshot.h"

CaptureSnapshot::CaptureSnapshot() {
  auto d3dDevice = util::uwp::CreateD3DDevice();
  auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
  m_device = CreateDirect3DDevice(dxgiDevice.get());
}

template<typename T>
T waitGet(winrt::IAsyncOperation<T> async)
{
  while (async.Status() == winrt::AsyncStatus::Started) {
    ::Sleep(1);
  }
  return async.GetResults();
}

Image*
CaptureSnapshot::takeImage(HWND hWnd) {
  winrt::IDirect3DSurface surface = waitGet(takeAsync(hWnd));
  if (!surface) return nullptr;

  winrt::SoftwareBitmap img = waitGet(winrt::SoftwareBitmap::CreateCopyFromSurfaceAsync(surface));

  auto buffer = img.LockBuffer(winrt::BitmapBufferAccessMode::ReadWrite);
  auto bufferByteAccess = buffer.CreateReference().as<IMemoryBufferByteAccess>();
  BYTE* data = nullptr;
  UINT32 capacity = 0;
  winrt::check_hresult(bufferByteAccess->GetBuffer(&data, &capacity));
  byte* samples = new byte[capacity];
  memcpy(samples, data, capacity);
  //img.CopyToBuffer(buffer);
  Image* image = new Image();
  image->width = img.PixelWidth();
  image->height = img.PixelHeight();
  image->samples = samples;
  return image;
}
winrt::IAsyncOperation<winrt::IDirect3DSurface>
CaptureSnapshot::takeAsync(HWND hWnd) {
  auto item = util::CreateCaptureItemForWindow(hWnd);
  if (!item) {
    co_return nullptr;
  }

  auto d3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
  winrt::com_ptr<ID3D11DeviceContext> d3dContext;
  d3dDevice->GetImmediateContext(d3dContext.put());

  // Creating our frame pool with CreateFreeThreaded means that we 
  // will be called back from the frame pool's internal worker thread
  // instead of the thread we are currently on. It also disables the
  // DispatcherQueue requirement.
  auto framePool = winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(
    m_device,
    winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized,
    1,
    item.Size());
  auto session = framePool.CreateCaptureSession(item);

  auto completion = completion_source<winrt::IDirect3DSurface>();
  framePool.FrameArrived([session, d3dDevice, d3dContext, &completion](auto& framePool, auto&)
    {
      auto frame = framePool.TryGetNextFrame();
      auto frameTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

      // Make a copy of the texture
      D3D11_TEXTURE2D_DESC desc = {};
      frameTexture->GetDesc(&desc);
      // Clear flags that we don't need
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      winrt::com_ptr<ID3D11Texture2D> textureCopy;
      winrt::check_hresult(d3dDevice->CreateTexture2D(&desc, nullptr, textureCopy.put()));
      d3dContext->CopyResource(textureCopy.get(), frameTexture.get());

      auto dxgiSurface = textureCopy.as<IDXGISurface>();
      auto result = CreateDirect3DSurface(dxgiSurface.get());

      // End the capture
      session.Close();
      framePool.Close();

      // Complete the operation
      completion.set(result);
    });

  session.StartCapture();
  co_return co_await completion;
}