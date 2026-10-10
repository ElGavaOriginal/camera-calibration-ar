// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>

/*
This program uses Harris to detect and track robust features in the video 
feed from the camera.  The program initializes the Harris feature detector and descriptor, and then in
each frame of the video feed, it detects keypoints and computes descriptors. The detected keypoints 
are drawn on the video feed
*/

// Generates a unique filename for saving feature images
static std::string makeFeatureFilename(const std::string& folder) {
    namespace fs = std::filesystem;

    if (!fs::exists(folder)) {
        fs::create_directories(folder);
    }
    int index = 0;
    while (true) {
        std::string filename = folder + "/features_" + std::to_string(index) + ".png";
        if (!fs::exists(filename)) {
            return filename;
        }
        index++;
    }
}


int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        return 1;
    }

    // Folder to save feature images
    const std::string featureFolder = "./data/features";

    // Parameters for Harris corner detection
    int blockSize = 2;
    int apertureSize = 3;
    double k = 0.04;
    int threshold = 100;

    // Print controls for the user
    std::cout << "Controls:\n";
    std::cout << "  u   = threshold up\n";
    std::cout << "  d   = threshold down\n";
    std::cout << "  Esc = exit program\n";
    std::cout << "  s   = save snapshot\n";

    while (true) {
        cv::Mat frame, gray;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not capture frame." << std::endl;
            break;
        }

        // Converts image to grayscale for corner detection
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        
        // Detects corners using Harris corner detection
        cv::Mat harrisResponse;
        cv::cornerHarris(gray, harrisResponse, blockSize, apertureSize, k);
        
        // Normalizes the corner response for display
        cv::Mat responseNorm;
        cv::normalize(harrisResponse, responseNorm, 0, 255, cv::NORM_MINMAX, CV_32FC1);

        // Draws detected corners on the original frame
        cv::Mat display = frame.clone();

        // Loops through the corner response and draws circles on the detected corners
        for (int y = 0; y < responseNorm.rows; y++) {
            for (int x = 0; x < responseNorm.cols; x++) {
                if ((int)responseNorm.at<float>(y, x) > threshold) {
                    cv::circle(display, cv::Point(x, y), 3, cv::Scalar(0, 0, 255), 1);
                }
            }
        }

        // Displays the current threshold on the video feed and shows the video feed with detected corners
        std::ostringstream text;
        text << "Harris threshold = " << threshold;
        cv::putText(display, text.str(), 
                    cv::Point(20, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 
                    0.8, 
                    cv::Scalar(0, 255, 0), 
                    2);

        // Displays controls onto the video feed
        cv::putText(display, "u/d: threshold s: save Esc: exit",
                    cv::Point(20, display.rows - 20),
                    cv::FONT_HERSHEY_COMPLEX, 0.6, 
                    cv::Scalar(255, 255, 255), 
                    2);

        cv::imshow("Robust Features - Harris Corners", display);
        char key = static_cast<char>(cv::waitKey(30));

        if (key == 'u' && threshold < 255) threshold += 5;
        if (key == 'd' && threshold > 0) threshold -= 5;
        if (key == 's' || key == 'S') {
            std::string filename = makeFeatureFilename(featureFolder);
            if (cv::imwrite(filename, display)) {
                std::cout << "Saved snaptshot: " << filename << std::endl;
            } else {
                std::cerr << "Error: Could not save snapshot: " << filename << std::endl;
            }
        }
        if (key == 27) break;     // Esc key to exit 
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
