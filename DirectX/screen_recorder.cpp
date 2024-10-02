#include "screen_recorder.h"
#include <iostream>

ScreenRecorder::ScreenRecorder(int width, int height)
{
    _w = width;
    _h = height;
}

ScreenRecorder::~ScreenRecorder()
{
    if (_duplicationManager)
        delete _duplicationManager;

    _duplicationManager = nullptr;

    if (_screenData != nullptr)
    {
        delete[] _screenData;
    }
}

char* ScreenRecorder::GetScreenData(long& screenDataSize)
{

    if (_duplicationManager == nullptr)
    {
        _duplicationManager = new DUPLICATIONMANAGER();

        if (!_duplicationManager->InitDupl())
        {
            std::cout << _duplicationManager->GetLastError() << std::endl;
        }
    }

    int triesLeft = 3;
    bool captureDone = false;
    while (!captureDone && triesLeft > 0)
    {
        triesLeft--;

        void* frameData = nullptr;
        int actualRowPitch = 0, texW = 0, texH = 0;

        if (!_duplicationManager->GetFrame(&frameData, &actualRowPitch))
        {
            _duplicationManager->GetTextureDimensions(texW, texH);

            long index = 0;

            if (!IsScreenEmpty(frameData))
            {
                try
                {
                    long size = _h * _w * 3;
                    long bufferIndex = 0;
                    return (char*)frameData;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Error: " << e.what() << std::endl;
                }

                // Capture done
                captureDone = true;
                //stream.Flush();
            }

            if (_duplicationManager->FinishFrame())
            {
                //LogError(_duplicationManager->GetLastError());
            }
        }

    }
    return _screenData;
}

bool ScreenRecorder::IsScreenEmpty(void* pointer)
{
    auto srcPointer = (UINT32*)pointer;
    auto lenth = _w * _h;

    if (srcPointer[0] == 0 && srcPointer[lenth / 2] == 0 && srcPointer[lenth - 1] == 0)
    {
        return true;
    }
    return false;
}
