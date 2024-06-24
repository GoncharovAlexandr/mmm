#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

void findQuantiles(const cv::Mat& src, double lowQuantile, double highQuantile, uchar& lowValue, uchar& highValue) {
    std::vector<uchar> pixels;
    pixels.reserve(src.total());

    if (src.isContinuous()) {
        pixels.assign(src.datastart, src.dataend);
    }
    else {
        for (int i = 0; i < src.rows; ++i) {
            pixels.insert(pixels.end(), src.ptr<uchar>(i), src.ptr<uchar>(i) + src.cols);
        }
    }

    std::sort(pixels.begin(), pixels.end());

    size_t lowIdx = static_cast<size_t>(lowQuantile * pixels.size());
    size_t highIdx = static_cast<size_t>(highQuantile * pixels.size());

    lowValue = pixels[lowIdx];
    highValue = pixels[highIdx];
}

void autoContrast(cv::Mat& src, double lowQuantile, double highQuantile) {
    uchar lowValue, highValue;
    findQuantiles(src, lowQuantile, highQuantile, lowValue, highValue);

    src.forEach<uchar>([lowValue, highValue](uchar& pixel, const int* position) -> void {
        if (pixel < lowValue) {
            pixel = 0;
        }
        else if (pixel > highValue) {
            pixel = 255;
        }
        else {
            pixel = cv::saturate_cast<uchar>(255.0 * (pixel - lowValue) / (highValue - lowValue));
        }
        });
}

void autoContrastColorChannel(cv::Mat& src, double lowQuantile, double highQuantile) {
    std::vector<cv::Mat> channels;
    cv::split(src, channels);

    for (auto& channel : channels) {
        autoContrast(channel, lowQuantile, highQuantile);
    }

    cv::merge(channels, src);
}

void autoContrastColorCombined(cv::Mat& src, double lowQuantile, double highQuantile) {
    cv::Mat flat;
    cv::Mat reshaped = src.reshape(1, src.total()); 
    reshaped.copyTo(flat);

    uchar lowValue, highValue;
    findQuantiles(flat, lowQuantile, highQuantile, lowValue, highValue);

    src.forEach<cv::Vec3b>([lowValue, highValue](cv::Vec3b& pixel, const int* position) -> void {
        for (int i = 0; i < 3; ++i) {
            if (pixel[i] < lowValue) {
                pixel[i] = 0;
            }
            else if (pixel[i] > highValue) {
                pixel[i] = 255;
            }
            else {
                pixel[i] = cv::saturate_cast<uchar>(255.0 * (pixel[i] - lowValue) / (highValue - lowValue));
            }
        }
        });
}

int main() {
    cv::Mat image = cv::imread("image.jpg", cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Could not open or find the image" << std::endl;
        return -1;
    }

    // Поканальное автоконтрастирование
   // autoContrastColorChannel(image, 0.52, 0.7);

    // Или совместное автоконтрастирование
    autoContrastColorCombined(image, 0.2, 0.7);
    cv::imshow("Auto Contrast", image);
    cv::waitKey(0);

    return 0;
}
