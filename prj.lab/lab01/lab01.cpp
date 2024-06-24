#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat Gradient(int width, int height) {
    cv::Mat gradient(height, width, CV_8UC1);

    for (int i = 0; i < width; ++i) {
        uchar value = static_cast<uchar>(i * 255 / width);
        gradient.col(i).setTo(value);
    }

    return gradient;
}

void GC(cv::Mat& image, float gamma) {
    cv::Mat lut(1, 256, CV_8UC1);
    uchar* ptr = lut.ptr();

    for (int i = 0; i < 256; ++i) {
        ptr[i] = static_cast<uchar>(255 * pow(i / 255.0, gamma));
    }

    cv::LUT(image, lut, image);
}

int main(int argc, char* argv[]) {
    int s = 3;          
    int h = 30;         
    float gamma = 2.4;  

    if (argc >= 2) {
        s = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        h = std::atoi(argv[2]);
    }
    if (argc >= 4) {
        gamma = std::atof(argv[3]);
    }

    cv::Mat gradientImage = Gradient(h, s);

    cv::Mat gammaCorrectedImage = Gradient(h, s);
    GC(gammaCorrectedImage, gamma);

    cv::Mat combinedImage;
    cv::vconcat(gradientImage, gammaCorrectedImage, combinedImage);

    std::string filename = "combined_gradient.png";
    cv::imwrite(filename, combinedImage);
    std::cout << "Сохранено в файл: " << filename << std::endl;

    return 0;
}
