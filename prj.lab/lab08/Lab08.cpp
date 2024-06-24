#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using namespace cv;
using namespace std;

Vec3f sRGBtoLinearRGB(const Vec3b& sRGB) {
    Vec3f linRGB;
    for (int i = 0; i < 3; i++) {
        float channel = sRGB[i] / 255.0;
        linRGB[i] = (channel <= 0.04045) ? (channel / 12.92) : (pow((channel + 0.055) / 1.055, 2.4));
    }
    return linRGB;
}

void draw3DScatterPlot(const vector<Vec3f>& points) {
    int size = 500;
    Mat plot(size, size, CV_8UC3, Scalar(0, 0, 0));

    for (const Vec3f& point : points) {
        int x = static_cast<int>(point[0] * size);
        int y = static_cast<int>(point[1] * size);
        int z = static_cast<int>(point[2] * size);

        if (x >= 0 && x < size && y >= 0 && y < size && z >= 0 && z < size) {
            plot.at<Vec3b>(y, x)[0] = saturate_cast<uchar>(z);
            plot.at<Vec3b>(y, x)[1] = saturate_cast<uchar>(y);
            plot.at<Vec3b>(y, x)[2] = saturate_cast<uchar>(x);
        }
    }

    imshow("3D Scatter Plot", plot);
    waitKey(0);
}

int main() {
    const string imagePath = "test.jpg";

    Mat image = imread(imagePath);
    if (image.empty()) {
        cout << "Could not open or find the image: " << imagePath << endl;
        return -1;
    }

    vector<Vec3f> linRGBPoints;

    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            Vec3b sRGB = image.at<Vec3b>(y, x);
            Vec3f linRGB = sRGBtoLinearRGB(sRGB);
            linRGBPoints.push_back(linRGB);
        }
    }

    draw3DScatterPlot(linRGBPoints);

    return 0;
}
