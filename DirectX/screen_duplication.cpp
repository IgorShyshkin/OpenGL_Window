// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <iostream>
#include <string>
#include "screen_duplication.h"

using namespace std;

//
// Constructor sets up references / variables
//
DUPLICATIONMANAGER::DUPLICATIONMANAGER() : m_DeskDupl(nullptr),
m_AcquiredDesktopImage(nullptr),
m_CopiedDesktopImage(nullptr),
m_Device(nullptr),
m_Context(nullptr)

{
    RtlZeroMemory(&m_OutputDesc, sizeof(m_OutputDesc));
}

//
// Destructor simply calls CleanRefs to destroy everything
//
DUPLICATIONMANAGER::~DUPLICATIONMANAGER()
{
    if (m_DeskDupl)
    {
        m_DeskDupl->Release();
        m_DeskDupl = nullptr;
    }

    if (m_AcquiredDesktopImage)
    {
        m_AcquiredDesktopImage->Release();
        m_AcquiredDesktopImage = nullptr;
    }

    if (m_CopiedDesktopImage)
    {
        m_CopiedDesktopImage->Release();
        m_CopiedDesktopImage = nullptr;
    }

    if (m_Context)
    {
        m_Context->Release();
        m_Context = nullptr;
    }

    if (m_Device)
    {
        m_Device->Release();
        m_Device = nullptr;
    }
}

//
// Failure handler
//
int DUPLICATIONMANAGER::ProcessFailure(LPCSTR Failure, HRESULT hr)
{
    std::string failureStr(Failure);
    failureStr = failureStr + "; HR = " + to_string(hr);

    m_LastError = failureStr.c_str();
    return -1;
}

//
// Initialize duplication interfaces
//
int DUPLICATIONMANAGER::InitDupl(UINT Output)
{
    HRESULT hr = S_OK;

    // Driver types supported
    D3D_DRIVER_TYPE DriverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    // Feature levels supported
    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };
    UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

    D3D_FEATURE_LEVEL FeatureLevel;

    // Create device
    for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
    {
        hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
            D3D11_SDK_VERSION, &m_Device, &FeatureLevel, &m_Context);
        if (SUCCEEDED(hr))
        {
            // Device creation success, no need to loop anymore
            break;
        }
    }
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to create device in InitializeDx", hr);
    }

    // Get DXGI device
    IDXGIDevice* DxgiDevice = nullptr;
    hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to QI for DXGI Device", hr);
    }

    // Get DXGI adapter
    IDXGIAdapter* DxgiAdapter = nullptr;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
    DxgiDevice->Release();
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to get parent DXGI Adapter", hr);
    }

    // Get output
    IDXGIOutput* DxgiOutput = nullptr;
    hr = DxgiAdapter->EnumOutputs(Output, &DxgiOutput);
    DxgiAdapter->Release();
    DxgiAdapter = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to get specified output in DUPLICATIONMANAGER", hr);
    }

    DxgiOutput->GetDesc(&m_OutputDesc);

    // QI for Output 1
    IDXGIOutput1* DxgiOutput1 = nullptr;
    hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
    DxgiOutput->Release();
    DxgiOutput = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to QI for DxgiOutput1 in DUPLICATIONMANAGER", hr);
    }

    // Create desktop duplication
    hr = DxgiOutput1->DuplicateOutput(m_Device, &m_DeskDupl);
    DxgiOutput1->Release();
    DxgiOutput1 = nullptr;
    if (FAILED(hr))
    {
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
        {
            return ProcessFailure("There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again.", hr);
        }
        return ProcessFailure("Failed to get duplicate output in DUPLICATIONMANAGER", hr);
    }

    return 0;
}


//
// Get next frame and write it into Data
//
int DUPLICATIONMANAGER::GetFrame(void** screenFrameData, int * rowPitch)
{
    IDXGIResource* DesktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;

    // Get new frame
    HRESULT hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        return ProcessFailure("Wait timeout", hr);
        return 0;
    }

    if (FAILED(hr))
    {
        return ProcessFailure("Failed to acquire next frame in DUPLICATIONMANAGER", hr);
    }

    // If still holding old frame, destroy it
    if (m_AcquiredDesktopImage)
    {
        m_AcquiredDesktopImage->Release();
        m_AcquiredDesktopImage = nullptr;
    }

    // QI for IDXGIResource
    hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_AcquiredDesktopImage));
    DesktopResource->Release();
    DesktopResource = nullptr;
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to QI for ID3D11Texture2D from acquired IDXGIResource in DUPLICATIONMANAGER", hr);
    }

    if (!m_CopiedDesktopImage)
    {
        D3D11_TEXTURE2D_DESC texture_Descr = {};
        m_AcquiredDesktopImage->GetDesc(&texture_Descr);

        this->_w = texture_Descr.Width;
        this->_h = texture_Descr.Height;

        D3D11_TEXTURE2D_DESC TexDesc;
        TexDesc.Width = texture_Descr.Width;
        TexDesc.Height = texture_Descr.Height;
        TexDesc.MipLevels = texture_Descr.MipLevels;
        TexDesc.ArraySize = texture_Descr.ArraySize;
        TexDesc.Format = texture_Descr.Format; // B8G8R8A8_UNorm
        TexDesc.SampleDesc = texture_Descr.SampleDesc;
        TexDesc.BindFlags = 0;
        TexDesc.MiscFlags = 0;
        TexDesc.Usage = D3D11_USAGE_STAGING;
        TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        m_Device->CreateTexture2D(&TexDesc, NULL, &m_CopiedDesktopImage);
    }

    m_Context->CopyResource(m_CopiedDesktopImage, m_AcquiredDesktopImage);

    D3D11_MAPPED_SUBRESOURCE mappedTexture = {};
    hr = m_Context->Map(m_CopiedDesktopImage, 0, D3D11_MAP_READ, 0, &mappedTexture);
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to map texture", hr);
    }

    *screenFrameData = mappedTexture.pData;
    *rowPitch = mappedTexture.RowPitch;
    /*if (mappedTexture.pData)
    {
        byte* pixelPointer = (byte*)mappedTexture.pData;

        cout << (UINT)pixelPointer[0] << " " << (UINT)pixelPointer[1] << " " << (UINT)pixelPointer[2] << " " << (UINT)pixelPointer[3] << endl;
    }*/

    return 0;
}

//
// Get next frame and write it into Data
//
int DUPLICATIONMANAGER::FinishFrame()
{
    m_Context->Unmap(m_CopiedDesktopImage, 0);

    HRESULT hr = m_DeskDupl->ReleaseFrame();
    if (FAILED(hr))
    {
        return ProcessFailure("Failed to release frame in DUPLICATIONMANAGER", hr);
    }

    if (m_AcquiredDesktopImage)
    {
        m_AcquiredDesktopImage->Release();
        m_AcquiredDesktopImage = nullptr;
    }

    return 0;
}

std::string DUPLICATIONMANAGER::GetLastError()
{
    if (!m_LastError)
        return std::string();
    return std::string(m_LastError);
}

void DUPLICATIONMANAGER::GetTextureDimensions(int& w, int& h)
{
    w = this->_w;
    h = this->_h;
}