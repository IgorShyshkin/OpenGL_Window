#ifndef INA_SERVICES_SCREEN_DUPLICATION_H
#define INA_SERVICES_SCREEN_DUPLICATION_H

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <string>

//
// Handles the task of duplicating an output.
//
class DUPLICATIONMANAGER
{
public:
    DUPLICATIONMANAGER();
    ~DUPLICATIONMANAGER();

    int GetFrame(void** screenFrameData, int * rowPitch);
    int FinishFrame();
    int InitDupl(UINT Output = 0);

    std::string GetLastError();

    void GetTextureDimensions(int& w, int& h);
private:
    int ProcessFailure(LPCSTR Str, HRESULT hr);

    // vars
    ID3D11Device* m_Device;
    ID3D11DeviceContext* m_Context;
    IDXGIOutputDuplication* m_DeskDupl;
    ID3D11Texture2D* m_AcquiredDesktopImage;
    ID3D11Texture2D* m_CopiedDesktopImage;
    DXGI_OUTPUT_DESC m_OutputDesc;

    LPCSTR m_LastError;
    int _w, _h;
};

#endif /* INA_SERVICES_SCREEN_DUPLICATION_H */