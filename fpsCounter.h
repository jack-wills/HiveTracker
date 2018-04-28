#pragma once


class fpsCounter
{
public:
    fpsCounter();
	void updateFps();
    Mat printFPS(Mat image);
    ~fpsCounter();
private:
    int frameCounter;
    time_t timeStart;
	int fps;
	vector<int> fpsVec;
};

