// Ruben A. Salido
// CS5330: Computer Vision and Pattern Recognition
// Project 4: Camera Calibration and Augmented Reality
// Northeastern University | Spring 2026


#include "tree_object.hpp"
#include <vector>
#include <opencv2/calib3d.hpp>

/*
This function draws a simple tree object on the given frame using openCV's drawing functions.
The tree is positioned to the right of the house, and pushed back on the z axis to appear to be 
slightly behind the house.
*/


// Struct holds the edge information for the tree object
namespace {
    struct Edge {
        cv::Point3f a;      // Starting point of the edge
        cv::Point3f b;      // Ending point of the edge
        cv::Scalar color;
        int thickness;
    };

    // Projects and draws edge from 3D to 2D using the camera's parameters
    void drawProjectedEdge(cv::Mat& frame, 
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
}

void drawTreeObject(cv::Mat& frame,
                    const cv::Mat& rvec,
                    const cv::Mat& tvec,
                    const cv::Mat& cameraMatrix,
                    const cv::Mat& distCoeffs) {
    
    // Positions the tree to the left of the house
    float cx = -0.2f;
    float cy = -0.7f;
    float cz = -4.5f;

    // Defines the edges of the tree (tree outer shape)
    cv::Point3f tip(cx, cy - 4.0f, cz);
    cv::Point3f l1(cx - 0.5f, cy - 3.2f, cz);
    cv::Point3f r1(cx + 0.5f, cy - 3.2f, cz);

    cv::Point3f l2(cx - 0.7f, cy - 2.4f, cz);
    cv::Point3f r2(cx + 0.7f, cy - 2.4f, cz);
    
    cv::Point3f l3(cx - 0.9f, cy - 1.6f, cz);
    cv::Point3f r3(cx + 0.9f, cy - 1.6f, cz);
    
    // Inner cut lines for style
    cv::Point3f cut1a(cx - 0.30f, cy - 3.2f, cz);
    cv::Point3f cut1b(cx - 0.08f, cy - 3.2f, cz);
    cv::Point3f cut1c(cx + 0.08f, cy - 3.2f, cz);
    cv::Point3f cut1d(cx + 0.30f, cy - 3.2f, cz);

    cv::Point3f cut2a(cx - 0.55f, cy - 2.4f, cz);
    cv::Point3f cut2b(cx - 0.18f, cy - 2.4f, cz);
    cv::Point3f cut2c(cx + 0.18f, cy - 2.4f, cz);
    cv::Point3f cut2d(cx + 0.55f, cy - 2.4f, cz);

    cv::Point3f cut3a(cx - 0.80f, cy - 1.6f, cz);
    cv::Point3f cut3b(cx - 0.28f, cy - 1.6f, cz);
    cv::Point3f cut3c(cx + 0.28f, cy - 1.6f, cz);
    cv::Point3f cut3d(cx + 0.80f, cy - 1.6f, cz);

    // Trunk of the tree
    float trunkTopY = cy - 1.6f;
    cv::Point3f t0(cx - 0.15f, trunkTopY, cz);
    cv::Point3f t1(cx + 0.15f, trunkTopY, cz);
    cv::Point3f t2(cx + 0.15f, trunkTopY + 0.9f, cz);
    cv::Point3f t3(cx - 0.15f, trunkTopY + 0.9f, cz);

    cv::Scalar treeColor(0, 200, 0);        // Dark green color for the tree
    cv::Scalar trunkColor(40, 110, 170);    // Brown color for the trunk

    std::vector<Edge> edges = {
        // Tree edges
        {tip, l1, treeColor, 2},
        {tip, r1, treeColor, 2},
        {l1, l2, treeColor, 2},
        {r1, r2, treeColor, 2},
        {l2, l3, treeColor, 2},
        {r2, r3, treeColor, 2},
        {l3, r3, treeColor, 2},

        // Inner cuts
        {cut1a, cut1b, treeColor, 2},
        {cut1c, cut1d, treeColor, 2},
        {cut2a, cut2b, treeColor, 2},
        {cut2c, cut2d, treeColor, 2},
        {cut3a, cut3b, treeColor, 2},
        {cut3c, cut3d, treeColor, 2},

        // Trunk
        {t1, t2, trunkColor, 2},
        {t2, t3, trunkColor, 2},
        {t3, t0, trunkColor, 2}
    };
    for (const auto& e : edges) {
        drawProjectedEdge(frame, e.a, e.b, rvec, tvec, cameraMatrix, distCoeffs, e.color, e.thickness);
    }
}
