#pragma once
#include <functional>


class DataCollector
{
    constexpr static const char endOfFrameChar = '\n';
    char buffer[128];
    size_t idx = 0;
public:
    std::function<void (char* frame, size_t size)> OnFrame;

    void HandleData(const char* data, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            buffer[idx++] = data[i];
            if(idx >= 128)
                return;
            if(data[i] == endOfFrameChar)
            {
                if(OnFrame)
                    OnFrame(buffer, idx);
                idx = 0;
            }
        }
    }
};



