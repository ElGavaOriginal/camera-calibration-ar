// Ruben A. Salido
// CS5330: Computer Vision
// Project 4: Calibration and A.R.
// Spring 2026

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

struct CornerDetectionResult {
    bool found = false; // Indicates if corners were found
    std::vector<cv::Point2f> corners; // Vector to store detected corners
};

CornerDetectionResult detectChessboardCorners(
    const cv::Mat& frame_bgr,
    cv::Size patternSize, 
    bool refineSubPix = true
);
