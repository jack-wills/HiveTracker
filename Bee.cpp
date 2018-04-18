#include "stdafx.h"
#include "Bee.h"


RNG rng(12345);

Bee::Bee(int t, vector<Point> c): contour(c), tag(t), track(4) {
	trackPointer = 0;
	color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	frameCount = 0;
	updated = 1;
	velocityX = 0;
	velocityY = 0;
	convexHull(Mat(contour), hull, false);
	moment = moments(contour, false);
	center = Point2f(moment.m10 / moment.m00, moment.m01 / moment.m00);
	for (int i = 0; i < track.size(); i++) {
		track[i] = center;
	}
	previousCenter = center;
	searchRadius = 25;
	updateBee(contour);
	absVelocity = 300;
}

bool Bee::endFrame() {
	if (!updated) {
		frameCount++;
		if (frameCount > 2) {
			return 1;
		}
	}
	else {
		frameCount = 0;
	}
	updated = 0;
	return 0;
}


Point2f Bee::getPrediction(void) {
	return nextPrediction;
}

vector<Point> Bee::getPredictionArea(void) {
	return nextPredictionArea;
}

int Bee::getTag(void) {
	return tag;
}

bool Bee::beeUpdated(void) {
	return updated;
}

float Bee::getAbsVelocity() {
	return absVelocity;
}

Point Bee::getCenter() {
	return center;
}

Point Bee::getPreviousCenter() {
	return previousCenter;
}

void Bee::updateBee(vector<Point> newContour) {
	previousCenter = center;
	contour = newContour;
	moment = moments(contour, false);
	convexHull(Mat(contour), hull, false);
	center = Point2f(moment.m10 / moment.m00, moment.m01 / moment.m00);
	track[trackPointer] = center;
	trackPointer = (trackPointer + 1) % track.size();
	updated = 1;
	velocityX = center.x - previousCenter.x;
	velocityY = center.y - previousCenter.y;
	if (velocityX > 75) {
		velocityX = 75;
	}
	if (velocityX < -75) {
		velocityX = -75;
	}
	if (velocityY > 75) {
		velocityY = 75;
	}
	if (velocityY < -75) {
		velocityY = -75;
	}
	//cout << "Vel for bee " << tag << " = " << velocityX << ", " << velocityY << endl;
	absVelocity = sqrt(velocityX*velocityX +velocityY*velocityY);
	//cout << "Uncertainty for bee " << tag << " = " << uncertainty << endl;
	nextPrediction = Point2f(center.x + velocityX, center.y + velocityY);
}
Mat Bee::printBee(Mat image) {
	if (!updated) {
		drawContours(image, vector<vector<Point> >(1, contour), 0, color, 1);
		line(image, center, nextPrediction, color, 2);
		vector<Point> printTrack(track.size());
		for (int i = 0; i < track.size(); i++) {
			printTrack[i] = track[(trackPointer + i) % track.size()];
		}
		polylines(image, printTrack, 0, color, 2, CV_AA);
		drawContours(image, vector<vector<Point> >(1, hull), 0, color, 2);
		putText(image, to_string(tag), Point(center.x - 5, center.y + 10), FONT_HERSHEY_TRIPLEX, 1, color);
	}
	return image;
}

Bee::~Bee()
{
}
