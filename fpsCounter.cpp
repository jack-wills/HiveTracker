#include "stdafx.h"
#include "fpsCounter.h"


fpsCounter::fpsCounter()
{
	timeStart = time(0);
	fps = 0;
	frameCounter = 0;
}

void fpsCounter::updateFps() {
	if (time(0) - timeStart >= 1) {
		fps = frameCounter;
		cout << "FPS = " << fps << endl;
		frameCounter = 0;
		timeStart = time(0);
	}
	frameCounter++;
}

Mat fpsCounter::printFPS(Mat image) {
	putText(image, to_string(fps), Point(100, 100), 0, 2, Scalar(255, 255, 255));
	return image;
}

fpsCounter::~fpsCounter()
{
}
