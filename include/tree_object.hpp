// Ruben A. Salido
// CS5330: Computer Vision
// Project 4: Calibration and A.R.
// Spring 2026

#ifndef TREE_OBJECT_HPP
#define TREE_OBJECT_HPP

#include <opencv2/opencv.hpp>

void drawTreeObject(cv::Mat& frame, 
                    const cv::Mat& rvec,
                    const cv::Mat& tvec,
                    const cv::Mat& cameraMatrix,
                    const cv::Mat& distCoeffs);

#endif