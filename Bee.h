#pragma once

using namespace std;
using namespace cv;

class Bee
{
public:
	Bee(int t, vector<Point> c);
	~Bee();
	bool endFrame();
	Point2f getPrediction(void);
	vector<Point> getPredictionArea(void);
	int getTag(void);
	bool beeUpdated(void);
	void updateBee(vector<Point> newContour);
	Mat printBee(Mat image); 
	float getAbsVelocity(void);
	Point getCenter();
	Point getPreviousCenter();

private:
	float absVelocity;
	char frameCount;
	bool updated;
	int searchRadius;
	Point2f nextPrediction;
	vector<Point> nextPredictionArea;
	float velocityX;
	float velocityY;
	Point2f previousCenter;
	vector<Point> contour;
	vector<Point> track;
	int trackPointer;
	vector<Point> hull;
	Point2f center;
	Moments moment;
	int tag;
	Scalar color;
};
