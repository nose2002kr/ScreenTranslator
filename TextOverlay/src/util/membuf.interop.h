#pragma once
#include <inspectable.h>

MIDL_INTERFACE("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")
IMemoryBufferByteAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetBuffer(BYTE * *value, UINT32 * capacity) = 0;
};
