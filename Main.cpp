// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int findScaleFactor(void);
void backgroundSubtraction(UMat &input, UMat &output, Ptr<BackgroundSubtractor> &bgSub, int scale);
void fillMask(UMat &input, UMat &output);
void thresholdBee(UMat &input, UMat &output, UMat &background);
void reduceContours(vector<vector<Point>> &contours, vector<vector<Point>> &contoursReduced, int scale);
void matchContoursToBees(vector<vector<Point>> &contours, vector<Bee> &beeArray, vector<int> &contourUsed);
void createNewBees(vector<vector<Point>> &contours, vector<Bee> &beeArray, vector<int> &contourUsed, unordered_set<int> &tags);
void clearLostBees(vector<Bee> &beeArray, unordered_set<int> &tags);
void printBees(Mat output, vector<Bee> &beeArray);

//Different sized kernals for erosion and dilation
Mat element3x3 = getStructuringElement(MORPH_RECT, Size(3, 3));
Mat element5x5 = getStructuringElement(MORPH_RECT, Size(5, 5));
Mat element7x7 = getStructuringElement(MORPH_RECT, Size(7, 7));


vector<Bee> beeArray;
unordered_set<int> tags;
VideoCapture cap;
Ptr<BackgroundSubtractor> backgroundSubtractor;
fpsCounter fps;

int main(int argc, const char * argv[]) {
    //Initiate OpenCL
    ocl::setUseOpenCL(true);
    
    //Define file path for video, change to integer 0 for camera input
    cap = VideoCapture("/Users/Jack/Desktop/OpenCV Bees/bees.mp4");
    
    //Initiate KNN background subtractor
    backgroundSubtractor = createBackgroundSubtractorKNN(500, 10000.0, true);
    
    //Find the apropriate image scale factor for input
    const int scale = findScaleFactor();
    
    //Read first frame of input to get input image size
    UMat imgRaw;
    cap.read(imgRaw);
    
    //Initiate counter object
    Counter beeCounter(beeArray, imgRaw.cols, imgRaw.rows);
    
	while (cap.isOpened() && waitKey(1) != 27) {
        //Read frame from input video
        cap.read(imgRaw);
		if (imgRaw.empty()) {
			break; //Quit program if video has ended or camera disconnected
		}
		//imshow("Raw Image", imgRaw);
        
        //Isolate moving objects from image
        UMat movingObjects;
        backgroundSubtraction(imgRaw, movingObjects, backgroundSubtractor, scale);
        //imshow("Background", movingObjects);
        
        //Resize to double scale image
        UMat image;
        resize(imgRaw, image, imgRaw.size() / (scale / 2));

        //Fill background mask
        UMat backgroundFilled;
        fillMask(movingObjects, backgroundFilled);
        //imshow("Filled Background", backgroundFilled);

        //Threshold image for black and yellow colours
        UMat maskComb;
        thresholdBee(image, maskComb, backgroundFilled);
		
        //Restore mask to full size, ready for contours
        resize(maskComb, maskComb, imgRaw.size());
        //imshow("Combined Mask", maskComb);
        
        //Find the contours of the image
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(maskComb, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        
        //Function to remove too small and too large contours
        vector<vector<Point> > contoursReduced;
        reduceContours(contours, contoursReduced, scale);
        
        //Data association algorithm
        vector<int> contourUsed(contoursReduced.size(), -1);
        matchContoursToBees(contoursReduced, beeArray, contourUsed);
        
        //If a contour is still present and an appropriate bee cannot be Matched to it, create a new Bee object
        createNewBees(contoursReduced, beeArray, contourUsed, tags);
        
        //Convert from UMat to Mat for drawing functions
        Mat output;
        imgRaw.copyTo(output);
        
        //Print all active bee objects
        printBees(output, beeArray);
		//imshow("Bees", output);
        
        //Update and draw counter onto output
        beeCounter.updateCounter();
        output = beeCounter.drawCounter(output);
		//imshow("Counter", output);
        
        //Update and print FPS
        fps.updateFps();
        output = fps.printFPS(output);
        imshow("Output", output);

		//Delete any bees that have not been updated for a certain amount of frames (Amount defined in Bee class)
		clearLostBees(beeArray, tags);
        
        //Uncomment when step through is required
        //while(waitKey(0) == -1);
    }
    return 0;
}

//Dynamically scale input
int findScaleFactor(void) {
    UMat input;
    cap.read(input);
    return int(float(input.cols) / 160.0f);
}

//Function to isolate moving objects from image
void backgroundSubtraction(UMat &input, UMat &output, Ptr<BackgroundSubtractor> &bgSub, int scale) {
    UMat image;
    resize(input, image, input.size() / scale);
    
    bgSub->apply(image.clone(), output);
    threshold(output, output, 254, 255, THRESH_BINARY);
    
    resize(output, output, input.size() / (scale / 2));
    //imshow("Background", output);
    dilate(output, output, element3x3);
}

//Fill holes present in a mask using flood fill
void fillMask(UMat &input, UMat &output) {
    input.copyTo(output);
    floodFill(output, Point(0, 0), Scalar(255));
    bitwise_not(output, output);
    bitwise_or(input, input, output);
}

//Threshold image for yellow and black, and combine with background mask
void thresholdBee(UMat &input, UMat &output, UMat &Background) {
    UMat maskYellow, maskBlack, imgHsv;
    cvtColor(input, imgHsv, COLOR_BGR2HSV);
    Scalar lowerYellow = Scalar(12, 65, 40);
    Scalar upperYellow = Scalar(24, 255, 255);
    Scalar lowerBlack = Scalar(0, 0, 0);
    Scalar upperBlack = Scalar(255, 255, 40);
    inRange(imgHsv, lowerYellow, upperYellow, maskYellow);
    inRange(imgHsv, lowerBlack, upperBlack, maskBlack);
    UMat maskComb;
    bitwise_or(maskYellow, maskBlack, maskComb, Background);
    erode(maskComb, maskComb, element3x3);
    dilate(maskComb, maskComb, element5x5);
    fillMask(maskComb, output);
}

//Remove contours out of defined size range
void reduceContours(vector<vector<Point>> &contours, vector<vector<Point>> &contoursReduced, int scale) {
    
    for (int i = 0; i < contours.size(); i++)
    {
        if (contours[i].size() > 160 / (scale*scale) && contours[i].size() < 4800 / (scale*scale))
        {
            contoursReduced.push_back(contours[i]);
        }
    }
    contours.clear();
}

//Data association
void matchContoursToBees(vector<vector<Point>> &contours, vector<Bee> &beeArray, vector<int> &contourUsed) {
    vector<vector<int>> distances(beeArray.size(), vector<int>(contours.size(), -10000));
    vector<int> closestBee(contours.size(), -10000);
    vector<int> closestBeeIndex(contours.size(), 0);
    int closestContourIndex;
    int currentBeeIndex;
    int currentDistance;
    for (int i = 0; i < contours.size(); i++) {
        for (int j = 0; j < beeArray.size(); j++) {
            distances[j][i] = pointPolygonTest(contours[i], beeArray[j].getPrediction(), true); //Distance between current contour and current bee prediction location, positive numbers are inside
            if (distances[j][i] > closestBee[i] && distances[j][i] >= -500) {
                closestBee[i] = distances[j][i];
                closestBeeIndex[i] = j;
            }
        }
    }
    if (beeArray.size() > 0) {
        vector<bool> completedIndex(closestBeeIndex.size(), 0); //Bees array index that have been search, this is for a speed improvement
        for (int i = 0; i < contours.size(); i++) {
            while (completedIndex[i] == 0) {
                closestContourIndex = i; //Contour array index
				bool noMatchingBee = 1;
                if(!beeArray[closestBeeIndex[i]].beeUpdated()){
					noMatchingBee = 0;
                    currentBeeIndex = closestBeeIndex[i]; //Bees array index
                    currentDistance = closestBee[i]; //Distance from current contour to diffrent bees
                }else{
                    for(int j = 0; j < beeArray.size(); j++){
                        if(j != closestBeeIndex[i] && distances[j][i] > currentDistance){
							noMatchingBee = 0;
							currentDistance = distances[j][i];
                            currentBeeIndex = j; //Bees array index
                        }
                    }
                }
				if (noMatchingBee) {
					break;
				}
                for (int j = i + 1; j < contours.size(); j++) {
                    if (closestBeeIndex[j] == currentBeeIndex && completedIndex[j] == 0) { //
                        completedIndex[j] = 1;
                        currentDistance = max(currentDistance, closestBee[j]);
                        if (currentDistance == closestBee[j]) {
                            closestContourIndex = j;
                        }
                    }
                }
                if (closestContourIndex == i){
                    completedIndex[i] = 1;
                }
                if (currentDistance > -2 * beeArray[currentBeeIndex].getUncertainty()) { //Search area is based on current velocity
                    beeArray[currentBeeIndex].updateBee(contours[closestContourIndex]); //Recalculate values of position and uncertainty
                    beeArray[currentBeeIndex].updateKalman(); //Update objects kalman filter with new values
                    contourUsed[closestContourIndex] = 1;
                }
            }
        }
    }
    for (int i = 0; i < beeArray.size(); i++) {
        beeArray[i].predictKalman();
    }
}

//Function to creat new bee objects
void createNewBees(vector<vector<Point>> &contours, vector<Bee> &beeArray, vector<int> &contourUsed, unordered_set<int> &tags) {
    
    for (int i = 0; i < contours.size(); i++) {
        if (contourUsed[i] == -1) { //If the contour has not yet been used
            for (int tag = 0; tag < 1000; tag++) { //Find the smallest tag that is free
                if (tags.find(tag) == tags.end()) {
                    tags.insert(tag); //Set tag to being used
                    Bee newBee = Bee(tag, contours[i]); //Create new Bee object with contour and tag
                    beeArray.push_back(newBee); //Place Bee object into the array of bees
                    break; //Once a tag is found break out of tag loop
                }
            }
        }
    }
}


void clearLostBees(vector<Bee> &beeArray, unordered_set<int> &tags) {
    for (int i = 0; i<beeArray.size(); i++) {
        if (beeArray[i].endFrame()) { //Returns true if the bee has been inactive. See Bee class for full description
            tags.erase(beeArray[i].getTag()); //Free the tag so it can be used by other Bee objects
            beeArray.erase(beeArray.begin() + i); //Erase the Bee at location i from the Bees array
            i--; //Go back a memory location because an object will take the place of the erased one
        }
    }
    beeArray.shrink_to_fit(); //Reduce the memory size for the Bee array
}

void printBees(Mat output, vector<Bee> &beeArray) {
    for (int i = 0; i<beeArray.size(); i++) {
        output = beeArray[i].printBee(output); //Print all Bees
    }
}
