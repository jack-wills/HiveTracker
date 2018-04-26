#include "stdafx.h"
#include "fpsCounter.h"


fpsCounter::fpsCounter()
{
    timeStart = time(0);
    fps = 0;
    frameCounter = 0;
}

void fpsCounter::updateFps() {
    frameCounter++;
    if (time(0) - timeStart >= 1) {
        fps = frameCounter;
		fpsVec.push_back(fps);
		cout << fps << endl;
        frameCounter = 0;
        timeStart = time(0);
    }
}

Mat fpsCounter::printFPS(Mat image) {
    putText(image, to_string(fps), Point(100, 100), 0, 2, Scalar(255, 255, 255));
    return image;
}

void fpsCounter::printFps() {
	ofstream outputFile;
	outputFile.open("FPS.txt");
	for (int i = 0; i < fpsVec.size(); i++) {
		outputFile << fpsVec[i] << endl;
	}
	outputFile.close();
}

fpsCounter::~fpsCounter()
{
}


