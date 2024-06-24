#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::Mat img = cv::imread("input.jpg");
    
    cv::Mat lab;
    cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);

    std::vector <cv::Mat> channels;
    cv::split(lab, channels);

    cv::Scalar a_mean = cv::mean(channels[1]);
    cv::Scalar b_mean = cv::mean(channels[2]);

    channels[1] = channels[1] - a_mean[0] + 128;
    channels[2] = channels[2] - b_mean[0] + 128;

    cv::merge(channels, lab);

    cv::cvtColor(lab, img, cv::COLOR_Lab2BGR);

    cv::imwrite("output.jpg", img);

    return 0;
}
