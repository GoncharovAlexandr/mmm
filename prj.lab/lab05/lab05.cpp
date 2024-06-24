#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

#define squareHeight 99
#define squareWidth  99
#define radius squareHeight / 4

std::vector<int> objectColors{ 0, 127, 255 };

void addlayer(int rowNum, cv::Mat& image, int i, int j) {
    int squareColor = objectColors[(i + rowNum) % 3];
    int circleColor = objectColors[(i + 2) % 3];

    cv::Mat partObject(squareHeight, squareWidth, CV_8UC1, 255);
    cv::Rect uniquePart(squareWidth * i, squareHeight * rowNum, squareHeight, squareWidth);

    cv::rectangle(partObject,
        cv::Rect(0, 0, squareHeight, squareWidth),
        cv::Scalar(squareColor), -1);

    cv::circle(partObject,
        cv::Point(squareHeight / 2, squareWidth / 2), radius,
        cv::Scalar(circleColor), -1);

    partObject.copyTo(image(uniquePart));
}

void addObjects(cv::Mat& image) {
    for (int i = 0; i < objectColors.size(); i++) {
        int rowNum = 0;
        for (int j = 0; j < objectColors.size(); j++) {
            if (j != i) {
                addlayer(rowNum, image, i, j);
                rowNum++;
            }
        }
    }
}

cv::Mat mergeResults(cv::Mat& image, std::vector<cv::Mat>& processedImgVec) {
    cv::Mat mergedImage(image.rows * 2, image.cols, CV_32F, 255);

    for (int i = 0; i < processedImgVec.size(); i++) {
        cv::Rect img(0, i * image.rows, image.cols, image.rows);
        processedImgVec[i].copyTo(mergedImage(img));
    }

    return mergedImage;
}

cv::Mat getFilteredImg(cv::Mat& image, int variant) {
    std::vector<cv::Mat> processedImgVec;

    cv::Mat firstProcessed;
    cv::Mat secondProcessed;

    cv::Mat kernel;
    cv::Mat invertedKernel;

    image.convertTo(image, CV_32F);

    switch (variant) {
    case 1:
        kernel = (cv::Mat_<double>(2, 2) << -1, 1, -1, 1) / 4.0;
        invertedKernel = (cv::Mat_<double>(2, 2) << 1, 1, -1, -1) / 4.0;
    case 2:
        kernel = (cv::Mat_<double>(2, 2) << 1, 0, 0, -1) / 4.0;
        invertedKernel = (cv::Mat_<double>(2, 2) << 0, 1, -1, 0) / 4.0;
    }
    cv::filter2D(image, firstProcessed, CV_32F, kernel);
    cv::filter2D(image, secondProcessed, CV_32F, invertedKernel);

    processedImgVec.push_back(firstProcessed);
    processedImgVec.push_back(secondProcessed);

    cv::Mat fullresult = mergeResults(image, processedImgVec).clone();

    return fullresult;
}

int main() {
    cv::Mat image(squareHeight * 2, squareWidth * 3, CV_8UC1, 255);
    cv::Mat processedImage(image.rows * 2, image.cols, CV_32F, 255);

    int variant = 1;

    addObjects(image);
    cv::imwrite("res.jpeg", image);

    processedImage = getFilteredImg(image, variant);

    cv::normalize(processedImage, processedImage, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::imwrite("resitog.jpeg", processedImage);
    cv::imshow("Result", processedImage);

    cv::waitKey(0);

    return 0;
}