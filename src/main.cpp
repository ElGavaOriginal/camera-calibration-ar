// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "target_detect.hpp"


/*
This is the main entry point for the appplication. 
    1. The system is responsible for Detecting and Extracting target corners from an image. 
    2. Selecting images for calibration
    3. Calibrate the camera using the selected images and the detected corners.
    4. Calculate current position on the camera using the detected corners and the calibration data.
    5. Projecting Outside Corners or a 3D Axes. 
    6. Creating a Virtual Object. 
    7. Detecting the Robust Features in the image and tracking them across frames.

    The main file is responsible for orchestrating the flow of the application, calling the 
    necessary functions and handling user input:  
    1. It initializes the camera and sets up the necessary parameters for capturing video.
    2. It enters a while loop that continuously captures frames from the camera and processes them.
    3. Inside the loop, it calls the detection and extraction functions to find the corners in the current frame.
    4. Prints debugging information to the console. 
    5. Shows the processed framd and the handle keys for user interaction.

    The corner coordinate as well as the instructions will be displayed on the top left corner of 
    the video screen. 

    The program will also take a snapshot of the current frame when the user presses 's' and saves it to 
    /data/snapshot.jpg. The image is also annotated to include the coordinate of the first corner. 
    This allows the user to capture specific frames for later analysis or calibration.

    To calibrate the camera, the user can press 'c' to calibrate the camera once at least 5 images/frames are saved. 
    The user can also press 'w' to write instrinsics to a file after the calibration succeeds. 
    Finally, once the user is done with the application, they can press 'Esc' to exit the program. 

    Menu:
        s   = save snapshot
        c   = calibrate camera (requires at least 5 saved snapshots)
        w   = write calibration intrinsics to file (requires successful calibration)
        Esc = exit program
    
    Input:  
        1. Real-time video feed from the camera.
        2. User input via keyboard for selecting images, calibrating, and exiting the application.

    Output: 
        2. Console output for debugging information about the detected corners and user interactions.
*/


// Prints the number of corners detected and the coordinate of the first corner to console
void printDebugInfo(const std::vector<cv::Point2f>& corners) {
    std::cout << "Number of corners detected: " << corners.size() << std::endl;

    if (!corners.empty()) {
        std::cout << "First corner: ("
                  << corners[0].x << ", "
                  << corners[0].y << ")" << std::endl;
    }
}

// Draws the detected corners on the frame and prints the coordinate of the first corner on the top left corner of the video screen
void drawCorners(cv::Mat& frame,
                 const std::vector<cv::Point2f>& corners,
                 cv::Size patternSize,
                 bool found) {
    cv::drawChessboardCorners(frame, patternSize, corners, found);
}

// This helper function formats the corner coordinates for display on the status panel
std::string formatCorner(const cv::Point2f pt, int precision = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << "(" << pt.x << ", " << pt.y << ")";
    return out.str();
}

// This function builds a 3D point set for the chessboard corners based on the specified pattern size.
std::vector<cv::Point3f> buildChessboardPointSet(cv::Size patternSize) {
    std::vector<cv::Point3f> point_set;

    for (int row = 0; row < patternSize.height; row++) {
        for (int col = 0; col < patternSize.width; col++) {
            point_set.push_back(cv::Point3f(
                static_cast<float>(col), 
                static_cast<float>(-row), 
                0.0f
            ));
        }
    }
    return point_set;
}

// This function draws a status panel on the video feed to provide real-time feedback 
// to the user about the corner detection and calibration process.
void drawStatusPanel(cv::Mat& frame, 
                    bool foundNow, 
                    const std::vector<cv::Point2f>& currentCorners, 
                    bool lastSaveAccepted, 
                    const std::string& lastSavedFilename,
                    const std::vector<std::vector<cv::Point3f>>& point_list,
                    const std::vector<std::vector<cv::Point2f>>& corner_list,
                    const std::vector<cv::Point3f>& point_set,
                    int lastCornersSaved,
                    bool calibrationReady,
                    bool calibrationDone,
                    double reprojectionError) {
    cv::Mat overlay = frame.clone();
    cv::rectangle(overlay, cv::Rect(10, 10, 560, 380), cv::Scalar(0, 0, 0), -1);
    cv::addWeighted(overlay, 0.45, frame, 0.55, 0.0, frame);

    int x = 25;
    int y = 35;
    int lineGap = 26;

    auto putLine = [&](const std::string text, const cv::Scalar& color = cv::Scalar(255, 255, 255), double scale = 0.62) {
        cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, scale, color, 2);
        y += lineGap;
    };

    putLine(foundNow ? "Checkerboard found" : "Checkerboard not found", foundNow ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255));

    if (!currentCorners.empty()) {
        cv::circle(frame, currentCorners[0], 8, cv::Scalar(0, 255, 255), -1);
        putLine("First corner: " + formatCorner(currentCorners[0], 2), cv::Scalar(0, 255, 0));
    } else {
        putLine("First corner: (n/a)", cv::Scalar(0, 255, 255));
    }

    putLine("Corners detected: " + std::to_string(currentCorners.size()), cv::Scalar(0, 255, 0));

    putLine(lastSaveAccepted ? "Calibration image accepted: " : "Calibration image not yet saved", 
            lastSaveAccepted ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 255, 255));
    
    putLine("point_list size  = " + std::to_string(point_list.size()));
    putLine("corner_list size = " + std::to_string(corner_list.size()));
    putLine("points in set    = " + std::to_string(point_set.size()));
    putLine("corners saved    = " + std::to_string(lastCornersSaved));

    if (!lastSavedFilename.empty()) {
        putLine("Saved image: " + lastSavedFilename, cv::Scalar(255, 255, 0), 0.52);
    } else {
        putLine("Saved image: none yet", cv::Scalar(255, 255, 0), 0.52);
    }

    putLine(calibrationReady ? "Calibration ready: yes" : "Calibration ready: need >= 5 images.",
            calibrationReady ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 165, 255));
    
    if (calibrationDone) {
        putLine("Reprojection error: " + std::to_string(reprojectionError), cv::Scalar(255, 255, 0), 0.56);
    } else {
        putLine("Reprojection error: n/a", cv::Scalar(255, 255, 0), 0.56);
    }

    putLine("Press 's' to save  calibration image");
    putLine("Press 'c' to calibrate camera");
    putLine("Press 'w' to write calibration to file");
    putLine("Press 'Esc' to exit");
}

// Detects chessboard corners in the given frame and returns the result in a structured format
void printMatrix(const std::string& label, const cv::Mat& mat) {
    std::cout << label << ":\n" << mat << "\n" << std::endl;
}

// Runs the camera calibration process using the collected 3D-2D point correspondences
// Returns the calibration results: camera matrix, distortion coefficients, and reprojection error
bool runCalibration(
    const std::vector<std::vector<cv::Point3f>>& point_list,
    const std::vector<std::vector<cv::Point2f>>& corner_list,
    const cv::Size& imageSize,
    cv::Mat& camera_matrix,
    cv::Mat& distortion_coefficients,
    std::vector<cv::Mat>& rotations,
    std::vector<cv::Mat>& translations,
    double& reprojection_error
) {
    if (point_list.size() < 5 || corner_list.size() < 5) {
        std::cout << "Need at least 5 calibration frames before calibrating.\n";
        return false;
    }
    if (point_list.size() != corner_list.size()) {
        std::cout<< "Error: point_list and corner_list sizes do not match.\n";
        return false; 
    }
    camera_matrix = (cv::Mat_<double>(3, 3) <<
        1.0, 0.0, imageSize.width / 2.0,
        0.0, 1.0, imageSize.height / 2.0,
        0.0, 0.0, 1.0);

        // Common distortion model, 5 coefficients: k1, k2, p1, p2, k3
        distortion_coefficients = cv::Mat::zeros(5, 1, CV_64F);
        std::cout << "\n **BEFORE CALIBRATION**" << std::endl;
        printMatrix("Initial camera matrix", camera_matrix);
        printMatrix("Initial distortion coefficients", distortion_coefficients);
        int flags = cv::CALIB_FIX_ASPECT_RATIO;
        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 100, 1e-6);

        reprojection_error = cv::calibrateCamera(
            point_list, 
            corner_list,
            imageSize, 
            camera_matrix, 
            distortion_coefficients, 
            rotations, 
            translations, 
            flags, 
            criteria
        );

        std::cout << "\n **AFTER CALIBRATION**" << std::endl;
        printMatrix("Calibrated camera matrix", camera_matrix);
        printMatrix("Calibrated distortion coefficients", distortion_coefficients);
        std::cout << "Final reprojection error: " << reprojection_error << "\n" << std::endl;

        return true;
}

// This function writes the calibrated camera intrinsics and reprojection error to a file
bool writeCalibrationToFile(
    const std::string& filename,
    const cv::Mat& camera_matrix,
    const cv::Mat& distortion_coefficients,
    double reprojection_error
) {
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        std::cout << "Error: could not open file for writing: " << filename << std::endl;
        return false;
    }

    fs << "camera_matrix" << camera_matrix;
    fs << "distortion_coefficients" << distortion_coefficients;
    fs << "reprojection_error" << reprojection_error;
    fs.release();
    std::cout << "Calibration parameters written to: " << filename << std::endl;
    return true;
}



int main() {

    // Initialize the camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        return 1;
    }

    // Print confirmation of startup and menu for the user
    std::cout << "Starting camera feed and corner detection..." << std::endl;
    std::cout << "Menu options:\n"
              << "  s   = save snapshot\n"
              << "  c   = calibrate camera\n"
              << "  w   = write intrinsics to file\n"
              << "  Esc = exit program\n"
              << std::endl;
    std::cout << "Snapshots will be saved to:\n"
              << "./data/snapshots/" << std::endl;

    // patternSize = number of inner corners per row and column
    cv::Size patternSize(7, 6);

    // Directory to save snapshots and a counter for naming the files
    const std::string snapshotDir = "./data/snapshots/";
    const std::string calibrationFilename = "./data/calibration/calibration_result.yml";
    int snapshotCount = 0;

    // Calibration storage
    std::vector<cv::Point3f> point_set = buildChessboardPointSet(patternSize);
    std::vector<std::vector<cv::Point3f>> point_list;
    std::vector<std::vector<cv::Point2f>> corner_list;

    // Save the last successful corner detection result for calibration
    cv::Mat lastGoodFrame;
    std::vector<cv::Point2f> lastGoodCorners;
    bool haveLastGoodDetection = false;
    bool lastSaveAccepted = false;
    int lastCornerSaved = 0;
    std::string lastSavedFilename = "";

    // Calibration results
    cv::Mat camera_matrix;
    cv::Mat distortion_coefficients;
    std::vector<cv::Mat> rotations;
    std::vector<cv::Mat> translations;
    double reprojectionError = -1.0;
    bool calibrationDone = false;

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Error: Could not capture frame." << std::endl;
            break;
        }

        CornerDetectionResult result = detectChessboardCorners(frame, patternSize, true);

        // Draws detected corners and prints the coordinate of the first corner on the top left corner of the video screen
        if (result.found) {
            drawCorners(frame, result.corners, patternSize, result.found);
            printDebugInfo(result.corners);

            // Update last good detection
            lastGoodFrame = frame.clone();
            lastGoodCorners = result.corners;
            haveLastGoodDetection = true;
        } 

        bool calibrationReady = (corner_list.size() >= 5 && point_list.size() >= 5);        
        drawStatusPanel(frame, 
            result.found, 
            result.corners, 
            lastSaveAccepted,
            lastSavedFilename,
            point_list,
            corner_list,
            point_set,
            lastCornerSaved,
            calibrationReady,
            calibrationDone,
            reprojectionError);

        cv::imshow("Target Corner Detection", frame);

        char key = static_cast<char>(cv::waitKey(30));

        // Saves a snapshot of the calibration frame when user presses 's'
        if (key == 's' || key == 'S') {
            if (haveLastGoodDetection) {
                // Saves the 2D corners from the last good frame detection
                corner_list.push_back(lastGoodCorners);

                // Saves matching 3D point set for calibration
                point_list.push_back(point_set);

                // Saves image used for calibration
                std::string filename = snapshotDir + "calibration_" + std::to_string(snapshotCount++) + ".png";
                std::vector<int> compressionParams = {cv::IMWRITE_PNG_COMPRESSION, 1};
                
                // Saves last good frame with detected corners drawn on it
                bool ok = cv::imwrite(filename, lastGoodFrame, compressionParams);

                if (ok) {
                    lastSaveAccepted = true;
                    lastSavedFilename = filename;
                    lastCornerSaved = static_cast<int>(lastGoodCorners.size());
                    std::cout << "Calibration image accepted." << std::endl;
                    std::cout << "  point_list size  = " << point_list.size() << std::endl;
                    std::cout << "  corner_list size = " << corner_list.size() << std::endl;
                    std::cout << "  points in set    = " << point_set.size() << std::endl;
                    std::cout << "  corners saved    = " << lastGoodCorners.size() << std::endl;

                } else {
                    lastSaveAccepted = false;
                    std::cout << "Error: no checkerboard detected or available to save." << std::endl;
                } 
            } else {
                lastSaveAccepted = false;
                std::cout << "Error: no successful checkerboard detected or available to save." << std::endl;
            }
        }

        // Runs calibration when user presses 'c'
        if (key  == 'c' || key == 'C') {
            bool ok = runCalibration(
                point_list,
                corner_list,
                frame.size(),
                camera_matrix,
                distortion_coefficients,
                rotations,
                translations,
                reprojectionError
            );

            calibrationDone = ok;

            if (ok) {
                std::cout << "Calibration completed successfully." << std::endl;
            } else {
                std::cout << "Error: Calibration failed or insufficient images." << std::endl;
            }
        }

        // Writes calibration results to file when user presses 'w'
        if (key == 'w' || key == 'W') {
            if (!calibrationDone) {
                std::cout << "Must first calibrate before parameters can be written to file." << std::endl;
            } else {
                writeCalibrationToFile(calibrationFilename, camera_matrix, distortion_coefficients, reprojectionError);
            }
        }

        // Exits the program when user presses 'Esc'
        if (key == 27) {
            std::cout << "Exiting application..." << std::endl;
            break;
        }
    }
    std::cout << "\nFinal calibration data summary:" << std::endl;
    std::cout << "  Saved corner sets: " << corner_list.size() << std::endl;
    std::cout << "  Saved point sets: " << point_list.size() << std::endl;

    if (calibrationDone) {
        std::cout << " \nFinal camera matrix:\n" << camera_matrix << std::endl;
        std::cout << " \nFinal distortion coefficients:\n" << distortion_coefficients << std::endl;
        std::cout << " \nFinal reprojection error:\n" << reprojectionError << std::endl;
    } else {
        std::cout << "\nCalibration was not completed successfully." << std::endl;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;

}
