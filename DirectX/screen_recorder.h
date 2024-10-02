#ifndef INA_SCREEN_RECORDER_H
#define INA_SCREEN_RECORDER_H

#include "screen_duplication.h"

class ScreenRecorder
{

private:
    DUPLICATIONMANAGER* _duplicationManager = nullptr;

    int _w, _h;

    char* _screenData = nullptr;
    long _screenDataSize = 0;

public:

    ScreenRecorder(int width, int height);

    ~ScreenRecorder();

    char* GetScreenData(long& screenDataSize);

private:

    bool IsScreenEmpty(void* pointer);

};

#endif /* INA_SCREEN_RECORDER_H */