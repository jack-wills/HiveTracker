#pragma once


class fpsCounter
{
public:
    fpsCounter();
    void updateFps(void);
    Mat printFPS(Mat image);
    ~fpsCounter();
private:
    int frameCounter;
    int fps;
    time_t timeStart;
};

