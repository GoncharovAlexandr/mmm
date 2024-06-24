#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <random>

cv::Mat makeImage(int l1, int l2, int l3) {
    cv::Mat test(256, 256, CV_8U, cv::Scalar(l1));
    cv::rectangle(test, cv::Point(10, 10), cv::Point(239, 239), l2, cv::FILLED);
    cv::circle(test, cv::Point(127, 127), 83, l3, cv::FILLED);
    return test;
}


cv::Mat drawHistogram(const cv::Mat& image) {
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };

    cv::Mat hist;
    cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    cv::normalize(hist, hist, 0, 230, cv::NORM_MINMAX);

    int histWidth = histSize;
    int histHeight = 256;
    int binWidth = cvRound((double)histWidth / histSize);
    cv::Mat histImage(histHeight, histWidth, CV_8UC1, 230);
    for (int i = 1; i < histSize; ++i) {
        cv::rectangle(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(hist.at<float>(i - 1))),
            cv::Point(binWidth * i, histHeight),
            0, cv::FILLED);
    }

    return histImage;
}


cv::Mat addNoise(const cv::Mat& image, double msq) {
    cv::Mat noisyImage = image.clone();

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(-msq, msq);

    for (int i = 0; i < noisyImage.rows; ++i) {
        for (int j = 0; j < noisyImage.cols; ++j) {
            double noise = distribution(generator);
            double newValue = noisyImage.at<uchar>(i, j) + noise;

            if (newValue < 0)
                newValue = 0;
            else if (newValue > 255)
                newValue = 255;

            noisyImage.at<uchar>(i, j) = static_cast<uchar>(newValue);
        }
    }

    return noisyImage;
}


int main(int argc, char** argv) {
    int levels[4][3] = {
            {0, 127, 255},
            {20, 127, 235},
            {55, 127, 200},
            {90, 127, 165}
    };

    cv::Mat result(256 * 7, 256 * 4, CV_8U, cv::Scalar(255));
    for (int i = 0; i < 4; i++) {
        cv::Mat test = makeImage(levels[i][0], levels[i][1], levels[i][2]);
        test.copyTo(result(cv::Rect(i * 256, 0, 256, 256)));

        double msq[3] = { 3.0, 7.0, 15.0 };
        for (int j = 0; j < 3; ++j) {
            cv::Mat noisyImage = addNoise(test, msq[j]);
            cv::Mat hist = drawHistogram(noisyImage);

            noisyImage.copyTo(result(cv::Rect(i * 256, j * 256 * 2 + 256, 256, 256)));
            hist.copyTo(result(cv::Rect(i * 256, j * 256 * 2 + 256 + 256, 256, 256)));
        }
    }

    cv::imshow("result", result);
    cv::waitKey(0);
}