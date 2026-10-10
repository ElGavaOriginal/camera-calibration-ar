// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include "sun_object.hpp"
#include <vector>
#include <opencv2/calib3d.hpp>

/*
This function draws a simple sun object on the given frame using openCV's drawing functions. 
The sun consists of a central circle and rays extending outward in 8 directions. The sun is positioned
to the upper left of the house in the scene. The rays are drawn as lines, and the center is drawn as a filled circle.
*/

namespace {
    void drawProjectedLine(cv::Mat& frame, 
                           const cv::Point3f& a,
                           const cv::Point3f& b,
                           const cv::Mat& rvec,
                           const cv::Mat& tvec,
                           const cv::Mat& cameraMatrix,
                           const cv::Mat& distCoeffs,
                           const cv::Scalar& color,
                           int thickness = 2) {
        std::vector<cv::Point3f>pts3D = {a, b};
        std::vector<cv::Point2f>pts2D;
        cv::projectPoints(pts3D, rvec, tvec, cameraMatrix, distCoeffs, pts2D);

        if (pts2D.size() == 2) {
            cv::line(frame, pts2D[0], pts2D[1], color, thickness);
        }
    }

    void drawProjectedPoint(cv::Mat& frame,
                            const cv::Point3f& p,
                            const cv::Mat& rvec,
                            const cv::Mat& tvec,
                            const cv::Mat& cameraMatrix,
                            const cv::Mat& distCoeffs,
                            const cv::Scalar& color,
                            int radius = 5) {
        std::vector<cv::Point3f>pts3D = {p};
        std::vector<cv::Point2f>pts2D;
        cv::projectPoints(pts3D, rvec, tvec, cameraMatrix, distCoeffs, pts2D);

        if (!pts2D.empty()) {
            cv::circle(frame, pts2D[0], radius, color, -1);
        }
    }
}

void drawSunObject(cv::Mat& frame,
                    const cv::Mat& rvec,
                    const cv::Mat& tvec,
                    const cv::Mat& cameraMatrix,
                    const cv::Mat& distCoeffs) {
    
    // Positions the sun to the upper right of the house
    cv::Point3f c(6.0f, -6.0f, -5.0f);
    float r = 0.6f;
    cv::Scalar sunColor(0, 255, 255);

    // Center point
    drawProjectedPoint(frame, c, rvec, tvec, cameraMatrix, distCoeffs, sunColor, 6);

    // Sun rays
    float innerDiag = 0.5f * r / std::sqrt(2.0f);
    float outerDiag = 1.5f * r / std::sqrt(2.0f);

    std::vector<std::pair<cv::Point3f, cv::Point3f>> rays = {
    {{c.x - 0.50f * r, c.y, c.z}, {c.x - 1.5f * r, c.y, c.z}},
    {{c.x + 0.50f * r, c.y, c.z}, {c.x + 1.5f * r, c.y, c.z}},
    {{c.x, c.y - 0.50f * r, c.z}, {c.x, c.y - 1.5f * r, c.z}},
    {{c.x, c.y + 0.50f * r, c.z}, {c.x, c.y + 1.5f * r, c.z}},

    {{c.x - innerDiag, c.y - innerDiag, c.z}, {c.x - outerDiag, c.y - outerDiag, c.z}},
    {{c.x + innerDiag, c.y - innerDiag, c.z}, {c.x + outerDiag, c.y - outerDiag, c.z}},
    {{c.x - innerDiag, c.y + innerDiag, c.z}, {c.x - outerDiag, c.y + outerDiag, c.z}},
    {{c.x + innerDiag, c.y + innerDiag, c.z}, {c.x + outerDiag, c.y + outerDiag, c.z}},
    };

    // Draws the rays of the sun
    for (const auto& ray : rays) {
        drawProjectedLine(frame, ray.first, ray.second, rvec, tvec, cameraMatrix, distCoeffs, sunColor, 2);
    }

    // Draws a circle at the center of the sun
        drawProjectedPoint(frame, c, rvec, tvec, cameraMatrix, distCoeffs, sunColor, 6);
}
