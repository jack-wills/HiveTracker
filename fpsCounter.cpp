#include "stdafx.h"
#include "fpsCounter.h"


fpsCounter::fpsCounter()
{
    timeStart = time(0);
    fps = 0;
    frameCounter = 0;
}

void fpsCounter::updateFps() {
    frameCounter++; //Increase frame counter
    if (time(0) - timeStart >= 1) { //After 1 second has passed since last record
        fps = frameCounter; //Frame counter after 1 second is equal to fps
		cout << fps << endl; //Print vector to console. TODO send this data over wifi, already done via SSH
        frameCounter = 0;
        timeStart = time(0); //Reset time
    }
}

Mat fpsCounter::printFPS(Mat image) {
    putText(image, to_string(fps), Point(100, 100), 0, 2, Scalar(255, 255, 255));
    return image;
}

fpsCounter::~fpsCounter()
{
}


