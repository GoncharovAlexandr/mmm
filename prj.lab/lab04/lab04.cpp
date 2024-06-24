#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <math.h>
#define _USE_MATH_DEFINES

using namespace cv;
using namespace std;

int blocksize = 197, constant = 7;

Mat detection(Mat start, Mat end) {
    int radius_min = 4;
    int radius_max = 25;
    double angle_min = -0.5 * acos(-1.0);
    double angle_max = +0.5 * acos(-1.0);
    vector<Vec3f> circles;
    imshow("Start", start);

    HoughCircles(start, circles, HOUGH_GRADIENT, 2, 50, constant, 10, 1, 25);
    for (size_t i = 0; i < circles.size(); i++)
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);

        circle(end, center, radius, Scalar(0, 255, 0), 3, 8, 0);
    }

    imshow("Start", start);
    imshow("Result1", end);
    waitKey(0);
    return end;
}

int main() {

    Mat rezult, adImg, noiseImg;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    float averageVal = 40;
    float sigma = 5;

    int indxBlure = 1, r = 25;
    for (int i = 0; i < 10; i++) {
        Mat image;
        Mat total(60, 60, CV_8UC1, Scalar(128));
        circle(total, Point(30, 30), r, 255, FILLED);

        total.copyTo(image);
        for (int k = 0; k < 9; k++) {
            Mat imgGray, imgBlure;
            GaussianBlur(image, imgBlure, Size(7, 7), indxBlure, 0);
            imgBlure.convertTo(imgBlure, -1, 1.0, 0);  
            vconcat(total, imgBlure, total);
            indxBlure = indxBlure + 2;
        }
        if (i == 0) total.copyTo(rezult);
        else hconcat(rezult, total, rezult);
        r = r - 2;
    }
    namedWindow("Start", WINDOW_NORMAL);
    namedWindow("Result1", WINDOW_NORMAL);

    Mat noise = Mat::zeros(rezult.size(), rezult.type());
    randu(noise, Scalar::all(0), Scalar::all(50));

    Mat startWithNoise = rezult + noise;

    if (blocksize % 2 == 0) {
        blocksize++;
        adaptiveThreshold(startWithNoise, adImg, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blocksize, constant);
    }
    else adaptiveThreshold(startWithNoise, adImg, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blocksize, constant);

    detection(adImg, rezult);

    imwrite("result.jpg", rezult);

    waitKey(0);

    return 0;
}
