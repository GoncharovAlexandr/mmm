#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <math.h>
#define _USE_MATH_DEFINES

using namespace cv;
using namespace std;

// Function to perform segmentation using a threshold
Mat segmentImage(Mat src) {
    Mat gray, segmented;
    // Convert to grayscale
    cvtColor(src, gray, COLOR_BGR2GRAY);
    // Apply threshold
    threshold(gray, segmented, 128, 255, THRESH_BINARY);
    return segmented;
}

// Function to calculate Intersection over Union (IoU)
double calculateIoU(Mat segmented, Mat reference) {
    // Ensure both images are binary
    threshold(segmented, segmented, 128, 255, THRESH_BINARY);
    threshold(reference, reference, 128, 255, THRESH_BINARY);

    // Calculate Intersection and Union
    Mat intersection, union_;
    bitwise_and(segmented, reference, intersection);
    bitwise_or(segmented, reference, union_);

    double intersectionArea = countNonZero(intersection);
    double unionArea = countNonZero(union_);

    return intersectionArea / unionArea;
}

int main() {
    Mat src = imread("result.jpg");
    if (src.empty()) {
        cout << "Could not open or find the image!" << endl;
        return -1;
    }

    Mat reference = imread("result.jpg", IMREAD_GRAYSCALE); 
    if (reference.empty()) {
        cout << "Could not open or find the reference image!" << endl;
        return -1;
    }

    Mat segmented = segmentImage(src);

    double IoU = calculateIoU(segmented, reference);
    cout << "Intersection over Union (IoU): " << IoU << endl;

    // Display results
    imshow("Source Image", src);
    imshow("Segmented Image", segmented);
    imshow("Reference Image", reference);
    waitKey(0);

    return 0;
}
