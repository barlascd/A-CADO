#include "vision/EdgeDetector.h"
#include <opencv2/opencv.hpp>

int detectEdges(const std::string& imagePath, const std::string& outPath) {
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) return -1;
    cv::Mat gray, edges;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 50, 150);
    cv::imwrite(outPath, edges);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    return (int)contours.size();
}
