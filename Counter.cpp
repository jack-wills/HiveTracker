#include "stdafx.h"
#include "Counter.h"


Counter::Counter(vector<Bee>& bees, int width, int height):beesArray(bees), imgWidth(width/2), imgHeight(height)
{
	counter = 0;
	color = Scalar(0, 0, 255);
}

void Counter::updateCounter() {
	for (int i = 0; i < beesArray.size(); i++) {
		if ((min(beesArray[i].getCenter().x, beesArray[i].getPreviousCenter().x) <= imgWidth)&& (max(beesArray[i].getCenter().x, beesArray[i].getPreviousCenter().x) >= imgWidth)) {
			if (beesArray[i].getCenter().x <= beesArray[i].getPreviousCenter().x) {
				counter++;
			}
			else {
				counter--;
			}
			color = Scalar(0, 255, 0);
		}
	}
}

Mat Counter::drawCounter(Mat image) {
	line(image, Point(imgWidth, 0), Point(imgWidth, imgHeight), color, 2);
	putText(image, to_string(counter), Point(imgWidth + 30, 80), 3, FONT_HERSHEY_DUPLEX , Scalar(0, 0, 255));
	color = Scalar(0, 0, 255);
	return image;
}

Counter::~Counter()
{
}