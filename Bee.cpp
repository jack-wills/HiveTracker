#include "stdafx.h"
#include "Bee.h"


RNG rng(12345);

Bee::Bee(int t, vector<Point> c): contour(c), tag(t), track(10) {
    trackPointer = 0;
    color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)); //Generate a random colour
    frameCount = 0;
    updated = 1;
    velocityX = 0;
    velocityY = 0;
    convexHull(Mat(contour), hull, false);
    moment = moments(contour, false);
    center = Point2f(moment.m10 / moment.m00, moment.m01 / moment.m00); //Center of contours from moments
    for (int i = 0; i < track.size(); i++) {
        track[i] = center;
    }
    previousCenter = center;
    searchRadius = 25;
    updateBee(contour);
    uncertainty = 100;
    nextPrediction = center;
    kalmanCounter = 0;
    
    state = Mat(6, 1, CV_32F);
    processNoise = Mat(6, 1, CV_32F);
    kalman.init(6, 4, 0, CV_32F); //initialisation of kalman object
    measurement = Mat(4, 1, CV_32F); //Definition of meaturement matrix
    measurement.setTo(Scalar(0)); //Clear
    
    //Below is the initial state matrices set to initial values
    kalman.statePre.at<float>(0) = center.x;
    kalman.statePre.at<float>(1) = center.y;
    kalman.statePre.at<float>(2) = previousCenter.x;
    kalman.statePre.at<float>(3) = previousCenter.y;
    kalman.statePre.at<float>(4) = 0;
    kalman.statePre.at<float>(5) = 0;
    kalman.statePost.at<float>(0) = center.x;
    kalman.statePost.at<float>(1) = center.y;
    kalman.statePost.at<float>(2) = previousCenter.x;
    kalman.statePost.at<float>(3) = previousCenter.y;
    kalman.statePost.at<float>(4) = 0;
    kalman.statePost.at<float>(5) = 0;
    
    //Transistion matrix defined in report
    kalman.transitionMatrix = (Mat_<float>(6, 6) << 1, 0, 0, 0, 1, 0,
                                                    0, 1, 0, 0, 0, 1,
                                                    1, 0, 0, 0, 0, 0,
                                                    0, 1, 0, 0, 0, 0,
                                                    1, 0, -1, 0, 0, 0,
                                                    0, 1, 0, -1, 0, 0);
    
    setIdentity(kalman.measurementMatrix);
    setIdentity(kalman.processNoiseCov, Scalar::all(1e-5));
    setIdentity(kalman.measurementNoiseCov, Scalar::all(1e-5));
    setIdentity(kalman.errorCovPost, Scalar::all(0.1));
    
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

float Bee::getUncertainty() {
    return uncertainty;
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
    
    //Calculate convex hull of contours
    convexHull(Mat(contour), hull, false);
    
    //Recalculate new center based on inputted contour
    center = Point2f(moment.m10 / moment.m00, moment.m01 / moment.m00);
    
    //Wrap around array that stores previous locations
    track[trackPointer] = center;
    trackPointer = (trackPointer + 1) % track.size();
    updated = 1;
    
    int diffX = center.x - nextPrediction.x;
    int diffY = center.y - nextPrediction.y;
    uncertainty = sqrt(diffX * diffX + diffY * diffY); //absolute error in previous prediction
    //normalise uncertainty
    if (uncertainty > 50){
        uncertainty = 50;
	}
	else if (uncertainty < 10) {
		uncertainty = 10;

	}
}
Mat Bee::printBee(Mat image) {
    
    if (updated) {
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

void Bee::updateKalman(void) {
    //Measurement matrix defined in report
    measurement(0) = center.x;
    measurement(1) = center.y;
    measurement(2) = previousCenter.x;
    measurement(3) = previousCenter.y;
    Mat mEstimated;
    mEstimated = kalman.correct(measurement); //Update kalman filter with new measurements
}

void Bee::predictKalman(void) {
    Mat mPrediction = kalman.predict(); //Get the next prediction from kalman filter
    if (kalmanCounter <= 2){ //Only during the first 3 interations while kalman filter initiates
        kalmanCounter++;
        nextPrediction = Point2f(2*center.x-previousCenter.x, 2*center.y-previousCenter.y); //Estimate first predictions with object velocity while kalman initiates
    }else{
        nextPrediction = Point2f(mPrediction.at<float>(0), mPrediction.at<float>(1)); //Place predictions into Point2f data type
    }
}

Bee::~Bee()
{
}