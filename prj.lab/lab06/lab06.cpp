#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <random>
#include <vector>

void add_noise(cv::Mat& image, double noise_level) {

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, noise_level);

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            uchar& pixel_value = image.at<uchar>(i, j);
            pixel_value += distribution(generator);
            if (pixel_value < 0) {
                pixel_value = 0;
            }
            else if (pixel_value > 255) {
                pixel_value = 255;
            }
        }
    }
}

std::vector<cv::Vec3f> detect_circles(const cv::Mat& image, double dp, double minDist, double param1, double param2, int minRadius, int maxRadius) {
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, minRadius, maxRadius);
    return circles;
}

std::pair<cv::Mat, cv::Mat> generated_images(std::vector<int> radiuses, std::vector<int> smoothing) {
    size_t sz = *std::max_element(radiuses.begin(), radiuses.end()) * 2 + 30;
    cv::Mat result(sz * radiuses.size(), sz * radiuses.size(), CV_8UC1, 70);
    cv::Mat binary_result(sz * radiuses.size(), sz * radiuses.size(), CV_8UC1);
    int n = radiuses.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            cv::Mat new_image(sz, sz, CV_8UC1, 70);
            cv::circle(new_image, cv::Point(sz / 2, sz / 2), radiuses[j], 110, -1);
            if (i > 0) {
                cv::Mat blurry_image;
                cv::GaussianBlur(new_image, new_image, cv::Size(smoothing[i], smoothing[i]), 0);
            }
            add_noise(new_image, 6);

            cv::Mat binary_new_image(sz, sz, CV_8UC1);
            cv::adaptiveThreshold(new_image, binary_new_image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 23, -8);
            binary_new_image.copyTo(binary_result(cv::Rect(sz * j, sz * i, sz, sz)));
            new_image.copyTo(result(cv::Rect(sz * j, sz * i, sz, sz)));
        }
    }
    return { result, binary_result };
}

struct Detection {
    int x;
    int y;
    int radius;
    bool is_true_positive;
};

std::vector<Detection> detect_circles_with_analysis(const cv::Mat& image, double dp, double minDist, double param1, double param2, int minRadius, int maxRadius, const std::vector<cv::Point2i>& ground_truth_centers, const std::vector<int>& ground_truth_radii) {
    std::vector<cv::Vec3f> circles = detect_circles(image, dp, minDist, param1, param2, minRadius, maxRadius);
    std::vector<Detection> detections;
    for (size_t i = 0; i < circles.size(); i++) {
        int x = cvRound(circles[i][0]);
        int y = cvRound(circles[i][1]);
        int radius = cvRound(circles[i][2]);
        bool is_true_positive = false;
        for (size_t j = 0; j < ground_truth_centers.size(); j++) {
            int dx = x - ground_truth_centers[j].x;
            int dy = y - ground_truth_centers[j].y;
            if (dx * dx + dy * dy <= (radius + ground_truth_radii[j]) * (radius + ground_truth_radii[j])) {
                is_true_positive = true;
                break;
            }
        }
        detections.push_back({ x, y, radius, is_true_positive });
    }
    return detections;
}

void draw_detections(cv::Mat& image, const std::vector<Detection>& detections) {
    for (size_t i = 0; i < detections.size(); i++) {
        cv::Point center(detections[i].x, detections[i].y);
        int radius = detections[i].radius;
        cv::Scalar color = detections[i].is_true_positive ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::circle(image, center, 3, color, -1, 8, 0);
        cv::circle(image, center, radius, color, 1, 8, 0);
    }
}

void draw_ground_truth(cv::Mat& image, const std::vector<cv::Point2i>& ground_truth_centers, const std::vector<int>& ground_truth_radii) {
    for (size_t i = 0; i < ground_truth_centers.size(); i++) {
        cv::Point center(ground_truth_centers[i].x, ground_truth_centers[i].y);
        int radius = ground_truth_radii[i];
        cv::circle(image, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
        cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 1, 8, 0);
    }
}

std::pair<double, double> calculate_froc_metrics(const std::vector<Detection>& detections) {
    int num_true_positives = 0;
    int num_false_positives = 0;
    for (size_t i = 0; i < detections.size(); i++) {
        if (detections[i].is_true_positive) {
            num_true_positives++;
        }
        else {
            num_false_positives++;
        }
    }
    double sensitivity = (double)num_true_positives / (num_true_positives + num_false_positives);
    double false_alarm_rate = (double)num_false_positives / (num_true_positives + num_false_positives);
    return { sensitivity, false_alarm_rate };
}

int main() {

    std::vector<int> radiuses = { 10, 20, 30, 40, 50 }; 
    std::vector<int> smoothing = { 3, 5, 7, 9, 11 }; 

    std::pair<cv::Mat, cv::Mat> images = generated_images(radiuses, smoothing);

    cv::Mat edges;
    cv::Canny(images.first, edges, 100, 200);

    
    std::vector<cv::Point2i> ground_truth_centers;
    std::vector<int> ground_truth_radii;
    for (int i = 0; i < radiuses.size(); i++) {
        for (int j = 0; j < radiuses.size(); j++) {
            int x = images.first.cols / 2 + j * (images.first.cols / radiuses.size()) - radiuses[j];
            int y = images.first.rows / 2 + i * (images.first.rows / radiuses.size()) - radiuses[i];
            ground_truth_centers.push_back(cv::Point2i(x, y));
            ground_truth_radii.push_back(radiuses[j]);
        }
    }

    std::vector<Detection> detections = detect_circles_with_analysis(edges, 1, 50, 100, 30, 0, 0, ground_truth_centers, ground_truth_radii);

    draw_detections(images.first, detections);
    draw_ground_truth(images.first, ground_truth_centers, ground_truth_radii);

    cv::imshow("Result Image", images.first); 
    cv::imshow("Binary Image", images.second);
    cv::waitKey(0);

    std::pair<double, double> froc_metrics = calculate_froc_metrics(detections);
    std::cout << "Sensitivity: " << froc_metrics.first << std::endl;
    std::cout << "False Alarm Rate: " << froc_metrics.second << std::endl;

    return 0;
}
