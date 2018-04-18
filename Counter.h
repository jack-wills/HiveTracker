#pragma once

class Counter
{
public:
	Counter(vector<Bee>& bees, int width, int height);
	void updateCounter(void);
	Mat drawCounter(Mat image);
	~Counter();
private:
	int counter;
	vector<Bee>& beesArray;
	const int imgWidth;
	const int imgHeight;
	Scalar color;
};

