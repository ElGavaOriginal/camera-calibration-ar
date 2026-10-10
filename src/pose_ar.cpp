// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <string>
#include "target_detect.hpp"
#include "tree_object.hpp"
#include "sun_object.hpp"
#include <opencv2/calib3d.hpp>

#include "target_detect.hpp"
#include "tree_object.hpp"
#include "sun_object.hpp"


/*
This file contains the implementation of steps 4, 5 and 6 of the project: "Calculate Current Position of the Camera", 
"Project Outside Corners of 3D Axes", and "Create a Virtual Object."  

*/


// Builds and prints a set of 3D points to a grid of the corner points of a chessboard 
static std::vector<cv::Point3f> buildChessboardPointSet(cv::Size patternSize, float squareSize = 1.0f) {
    std::vector<cv::Point3f> pts;
    for (int row = 0; row < patternSize.height; row++) {
        for (int col = 0; col < patternSize.width; col++) {
            pts.emplace_back(col * squareSize, -row * squareSize, 0.0f);
        }
    }
    return pts;
}

// Reads the camera calibration parameters from a file and returns a boolean. 
static bool readCalibration(const std::string& filename,
                            cv::Mat& cameraMatrix,
                            cv::Mat& distCoeffs,
                            double& reprojectionError) {
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr <<"Error: could not open the calibration file: " << filename << std::endl;
        return false;
    }

    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    fs["reprojection_error"] >> reprojectionError;
    fs.release();

    if (cameraMatrix.empty() || distCoeffs.empty()) {
        std::cerr <<"Error: camera matrix and/or distortion coefficients are missing in the calibration file: "
                  << filename << std::endl;
        return false;
    }
    return true;
}

// Draws the 3D axes on the image frame based on the rotation and 
// translation vectors, camera matrix, and distortion coefficients. 
static void drawAxes(cv::Mat&frame,                 // The image frame on which to draw the axes
                    const cv::Mat& rvec,            // Rotation vector obtained from solvePnP
                    const cv::Mat& tvec,               // Translation vector obtained from solvePnP
                    const cv::Mat& cameraMatrix,       // Camera intrinsic matrix obtained from calibration
                    const cv::Mat& distCoeffs,      // Distortion coefficients obtained from calibration
                    float axisLength = 2.0f) {      // Length of the axes to be drawn
    std::vector<cv::Point3f> axis3D = {
        {0, 0, 0},                                  // Origin point
        {axisLength, 0, 0},                         // +X
        {0, -axisLength, 0},                        // +Y negative because the image coordinate system has y pointing downwards
        {0, 0, -axisLength}                         // +Z negative because the camera looks along the positive Z-axis
    };
    std::vector<cv::Point2f> axis2D;               // Holds the projected 2D points of the axes
                                                    // Projects the 3D points to the 2D image points using the camera parameters
    cv::projectPoints(axis3D, rvec, tvec, cameraMatrix, distCoeffs, axis2D); 

    // Checks if the projection was successful and if there are 4 points to draw the axes
    if (axis2D.size() == 4) {
        cv::line(frame, axis2D[0], axis2D[1], cv::Scalar(0, 0, 255), 3);
        cv::line(frame, axis2D[0], axis2D[2], cv::Scalar(0, 255, 0), 3);
        cv::line(frame, axis2D[0], axis2D[3], cv::Scalar(255, 0, 0), 3);

        cv::putText(frame, "X", axis2D[1], cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
        cv::putText(frame, "y", axis2D[2], cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, "Z", axis2D[3], cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);
    }                                               // Draws the axes on the image frame with different colors for each axis and labels them.
}

// Establishes a colored edge structure to represent the edges of a virtual object in 3D space
struct ColoredEdge {cv::Point3f a; cv::Point3f b; cv::Scalar color;};

// Draws a virtual object on the image frame by projecting its 3D edges onto the 2D image plane using the camera parameters and the pose of the camera
static void drawVirtualObject(cv::Mat& frame,
                             const cv::Mat& rvec,
                             cv::Mat& tvec,
                             cv::Mat& cameraMatrix,
                             const cv::Mat& distCoeffs)
{

    // Defines front face of house frame (z = -2.0)
    cv::Point3f f0(2.0f, -1.5f, -2.0f);     // bottom left
    cv::Point3f f1(4.0f, -1.5f, -2.0f);     // bottom right
    cv::Point3f f2(4.0f, -3.5f, -2.0f);     // top right
    cv::Point3f f3(2.0f, -3.5f, -2.0f);     // top left
    cv::Point3f fa(3.0f, -4.6f, -3.0f);     // apex roof peak

    // Defines back face of the house frame (z = -4.0)
    cv::Point3f b0(2.0f, -1.5f, -4.0f); 
    cv::Point3f b1(4.0f, -1.5f, -4.0f);
    cv::Point3f b2(4.0f, -3.5f, -4.0f);
    cv::Point3f b3(2.0f, -3.5f, -4.0f);
    cv::Point3f ba(3.0f, -4.6f, -3.0f);     // back roof peak

    // Defines door and front face
    cv::Point3f d0(2.7f, -1.5f, -2.0f);
    cv::Point3f d1(3.3f, -1.5f, -2.0f);
    cv::Point3f d2(3.3f, -2.5f, -2.0f);
    cv::Point3f d3(2.7f, -2.5f, -2.0f);

    // - - - Label all points for debugging purposes - - -
    // - - - Each entry: {3D point, label string, text color} - - -
    struct LabeledPoint {cv::Point3f pt; std::string name; cv::Scalar color;};
    std::vector<LabeledPoint> labeledPts = {
        {f0, "f0", cv::Scalar(255, 200, 0)}, // light blue
        {f1, "f1", cv::Scalar(255, 200, 0)},
        {f2, "f2", cv::Scalar(255, 200, 0)},
        {f3, "f3", cv::Scalar(255, 200, 0)},
        {fa, "fa", cv::Scalar(255, 200, 0)},
        {b0, "b0", cv::Scalar(0, 200, 255)}, // orange
        {b1, "b1", cv::Scalar(0, 200, 255)},
        {b2, "b2", cv::Scalar(0, 200, 255)},
        {b3, "b3", cv::Scalar(0, 200, 255)},
        {ba, "ba", cv::Scalar(0, 200, 255)}, 
        {d0, "d0", cv::Scalar(0, 255, 100)}, // hot green
        {d1, "d1", cv::Scalar(0, 255, 100)},
        {d2, "d2", cv::Scalar(0, 255, 100)},
        {d3, "d3", cv::Scalar(0, 255, 100)},
    };

    for (const auto& lp : labeledPts) {
        std::vector<cv::Point3f> pts3D = {lp.pt};
        std::vector<cv::Point2f> pts2D;
        cv::projectPoints(pts3D, rvec, tvec, cameraMatrix, distCoeffs, pts2D);
        if (!pts2D.empty()) {
            cv::circle(frame, pts2D[0], 5, lp.color, -1);
            cv::putText(frame, lp.name, pts2D[0] + cv::Point2f(6, -6),
                        cv::FONT_HERSHEY_COMPLEX, 0.55, lp.color, 2);
        }
    }


    // Draws edges of the house and door
    struct Edge {cv::Point3f a; cv::Point3f b; cv::Scalar color;};

    // Defines the edges of the house by connecting the points and assigning different colors to different parts of the house
    std::vector<Edge> edges = {
        // front square
        {f0, f1, cv::Scalar(147, 20, 255)}, // hot pink
        {f1, f2, cv::Scalar(147, 20, 255)},
        {f2, f3, cv::Scalar(147, 20, 255)},
        {f3, f0, cv::Scalar(147, 20, 255)},

        // front roof
        {f3, fa, cv::Scalar (147, 20, 255)},
        {f2, fa, cv::Scalar (147, 20, 255)},

        // back square
        {b0, b1, cv::Scalar (147, 20, 255)},
        {b1, b2, cv::Scalar (147, 20, 255)},
        {b2, b3, cv::Scalar (147, 20, 255)},
        {b3, b0, cv::Scalar (147, 20, 255)},
        
        // back roof
        {b3, ba, cv::Scalar(147, 20, 255)},
        {b2, ba, cv::Scalar(147, 20, 255)},

        // connect front to back
        {f0, b0, cv::Scalar(147, 20, 255)},
        {f1, b1, cv::Scalar(147, 20, 255)},
        {f2, b2, cv::Scalar(147, 20, 255)},
        {f3, b3, cv::Scalar(147, 20, 255)},

        // ridge beam
        {fa, ba, cv::Scalar(147, 20, 255)},

        // door
        {d0, d1, cv::Scalar(0, 255, 100)},  // hot green
        {d1, d2, cv::Scalar(0, 255, 100)},
        {d2, d3, cv::Scalar(0, 255, 100)},
        {d3, d0, cv::Scalar(0, 255, 100)},
    };

    // Projects the 3D endpoints to the 2D image points, and draws the edges in color
    for (const auto& e : edges) {
        std::vector<cv::Point3f> pts3D = {e.a, e.b};
        std::vector<cv::Point2f> pts2D;
        cv::projectPoints(pts3D, rvec, tvec, cameraMatrix, distCoeffs, pts2D);
        if (pts2D.size() == 2) {
            cv::line(frame, pts2D[0], pts2D[1], e.color, 3);
        }
    }
}

static void drawProjectedOuterCorners(cv::Mat& frame, 
                                      const cv::Mat& rvec, 
                                      const cv::Mat& tvec,
                                      const cv::Mat& cameraMatrix,
                                      const cv::Mat& distCoeffs,
                                      cv::Size patternSize, 
                                      float squareSize) {
    // Defines the 3D coordinates of the outer corners of the chessboard
    float w = static_cast<float>(patternSize.width - 1) * squareSize;
    float h = static_cast<float>(patternSize.height - 1) * squareSize;

    std::vector<cv::Point3f> outer3D = {{0.0f, 0.0f, 0.0f}, {w, 0.0f, 0.0f}, {w, -h, 0.0f}, {0.0f, -h, 0.0f}};

    std::vector<cv::Point2f> outer2D;
    cv::projectPoints(outer3D, rvec, tvec, cameraMatrix, distCoeffs, outer2D);

    // Draws circles at the projected outer corners and connects them with lines to visualize the edges of the chessboard
    if (outer2D.size() == 4) {
        for (size_t i = 0; i < 4; i++) {
            cv::circle(frame, outer2D[i], 8, cv::Scalar(0, 0, 255), -1);
            cv::line(frame, outer2D[i], outer2D[(i + 1) % 4], cv::Scalar(255, 255, 0), 2);
        }
    }
}

// Generates a filenname to save the current frame with the projected features
static std::string makeScreenshotFilename(const std::string& folder) {
    namespace fs = std::filesystem;

    if (!fs::exists(folder)) {
        fs::create_directories(folder);
    }
    int index = 0;
    while (true) {
        std::string filename = folder + "/position_" + std::to_string(index) + ".png";
        if (!fs::exists(filename)) {
            return filename;
        }
        index++;
    }
}

int main() {
    // Defines the chessboard pattern size, the size of each square, and the path to the calibration file
    const cv::Size patternSize(7, 6);
    const float squareSize = 1.0f;
    const std::string calibrationFile = "./data/calibration/calibration_result.yml";
    const std::string screenshotFolder = "./data/position";

    // Holds the camera matrix, distortion coefficients, and reprojection error loaded from the calibration file
    cv::Mat cameraMatrix, distCoeffs;
    double reprojectionError = 0.0;

    if (!readCalibration(calibrationFile, cameraMatrix, distCoeffs, reprojectionError)) {
        return 1;
    }

    // Prints the loaded calibration parameters and instructions for the user
    std::cout << "Loaded Calibration from: " << calibrationFile << "\n";
    std::cout << "Reprojection Error: " << reprojectionError << "\n";
    std::cout << "Press ESC to quit.\n";

    // Initializes video capture from the default camera, and basic error handling if the camera cannot be opened
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: could ot open camera.\n";
        return 1;
    }

    // Builds the 3D point set for the chessboard corners
    std::vector<cv::Point3f> objectPoints = buildChessboardPointSet(patternSize, squareSize);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: empty frame.\n";
            break;
        }

        CornerDetectionResult result = detectChessboardCorners(frame, patternSize, true);

        if (result.found) {
            cv::drawChessboardCorners(frame, patternSize, result.corners, result.found);
            cv::Mat rvec, tvec;
            bool ok = cv::solvePnP(
                objectPoints, 
                result.corners, 
                cameraMatrix, 
                distCoeffs, 
                rvec,
                tvec,
                false,
                cv::SOLVEPNP_ITERATIVE
            );

            // Prints the rotation and translation vectors, draws the axes, projected corners, 
            // and virtual object on the image frame, and displays the translation vector as text on the image.
            if (ok) {std::cout << "rvec = " << rvec.t() << "\n";
                std::cout << "tvec = " << tvec.t() << std::endl;

                drawProjectedOuterCorners(frame, rvec, tvec, cameraMatrix, distCoeffs, patternSize, squareSize);
                drawAxes(frame, rvec, tvec, cameraMatrix, distCoeffs, 2.0f);
                drawVirtualObject(frame, rvec, tvec, cameraMatrix, distCoeffs);
                drawTreeObject(frame, rvec, tvec, cameraMatrix, distCoeffs);
                drawSunObject(frame, rvec, tvec, cameraMatrix, distCoeffs);

                std::ostringstream poseText;
                poseText << "tvec: [" 
                         << std::fixed 
                         << std::setprecision(2) 
                         << tvec.at<double>(0) 
                         << ", " 
                         << tvec.at<double>(1) 
                         << ", " 
                         << tvec.at<double>(2) 
                         << "]";
                cv::putText(frame, 
                            poseText.str(), 
                            cv::Point(20, 30), 
                            cv::FONT_HERSHEY_SIMPLEX, 
                            0.7, 
                            cv::Scalar(255, 255, 255),
                            2
                );

                std::ostringstream rotText;
                rotText << "rvec: ["
                        << std::fixed << std::setprecision(2)
                        << rvec.at<double>(0) << ", "
                        << rvec.at<double>(1) << ", "
                        << rvec.at<double>(2) << "]";
                cv::putText(frame,
                            rotText.str(),
                            cv::Point(20, 60),
                            cv::FONT_HERSHEY_SIMPLEX,
                            0.7,
                            cv::Scalar(255, 255, 255),
                            2
                );
            }
        }
        cv::putText(frame, "Press 's' to save PNG | Esc to quit",
                    cv::Point(20, frame.rows - 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(255, 255, 255), 2);

        cv::imshow("Pose + A.R.", frame);
        char key = static_cast<char>(cv::waitKey(30));
        if (key == 's' || key == 'S') {
            std::string filename = makeScreenshotFilename(screenshotFolder);
            if (cv::imwrite(filename, frame)) {
                std::cout << "Saved screenshot: " << filename << std::endl;
            } else {
                std::cerr << "Error: coud not save screenshot to: " << filename << std::endl;
            }
        }

        if (key == 27) {
            break;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
