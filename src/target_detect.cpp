// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include <opencv2/opencv.hpp>
#include <vector>
#include "target_detect.hpp"

/*
This file is responsible for detecting and extracting target corners from an image.
This file also ownx the OpenCV calls: findChessboardCorners and cornerSubPix, and 
uses a STRUCT to return the results of the corner detection. 

*/


CornerDetectionResult detectChessboardCorners(const cv::Mat& frame_bgr, cv::Size patternSize, bool refineSubPix) {
    CornerDetectionResult result;

    // Convert the input frame to grayscale
    cv::Mat gray;
    cv::cvtColor(frame_bgr, gray, cv::COLOR_BGR2GRAY);

    int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;  // source: https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html

    result.found = cv::findChessboardCorners(gray, patternSize, result.corners, flags);

    if (result.found && refineSubPix) {
        cv::cornerSubPix(gray, result.corners, cv::Size(11, 11), cv::Size(-1, -1), 
        cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
    }

    return result;
}
