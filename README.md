# Camera Calibration & Augmented Reality

**Computer Vision | C++ | OpenCV | Camera Geometry | Augmented Reality**

A computer vision project implementing an end-to-end camera calibration and marker-based augmented reality pipeline. The system detects checkerboard corners, estimates camera intrinsics, calculates the target's 3D pose, and projects virtual objects onto a live camera feed.

Developed for **CS5330: Computer Vision and Pattern Recognition** at Northeastern University (Spring 2026).

## Project Overview

This project consists of three C++ applications:

- **Camera Calibration (`detect_corners`):** Detects a 7 × 6 checkerboard pattern, collects calibration images, and calculates the camera's intrinsic matrix and lens distortion coefficients.
- **Augmented Reality (`pose_ar`):** Uses the calibrated camera parameters to estimate the checkerboard's position and orientation, then projects virtual 3D objects—including a wireframe house, tree, and sun—into the camera image.
- **Harris Corner Detection (`robust_features`):** Identifies distinctive image corners using the Harris corner response and visualizes them with adjustable detection thresholds.

## Augmented Reality Demonstration

The virtual scene is anchored to the checkerboard using camera pose estimation and 3D-to-2D projection.

![Augmented Reality Scene](data/position/position_15.png)

![Augmented Reality Scene from Another View](data/position/position_17.png)

## Camera Calibration

The system uses a checkerboard with **7 × 6 internal corners**, producing 42 feature points per calibration image.

Corner locations are refined to subpixel accuracy before being used to estimate the camera's intrinsic parameters and lens distortion.

![Checkerboard Corner Detection](data/snapshots/calibration_0.png)

### Calibration Results

| Metric | Result |
|---|---|
| Calibration target | 7 × 6 internal corners |
| Detected corners | 42 per image |
| Horizontal focal length | 1192.88 px |
| Vertical focal length | 1192.88 px |
| Principal point | (932.03, 593.80) px |
| Reprojection error | **0.5905 px** |

These results were obtained from the original calibration dataset and camera configuration.

## Camera Pose Estimation

Using `cv::solvePnP()`, the system estimates the rotation and translation of the checkerboard relative to the camera.

The resulting pose is used with `cv::projectPoints()` to project 3D coordinate axes and custom geometry into the camera image.

![Camera Pose and Projection](data/position/position_1.png)

## Harris Corner Detection

A separate application implements Harris corner detection using `cv::cornerHarris()`.

It identifies high-response corners in individual webcam frames and allows the user to adjust the detection threshold interactively.

![Harris Corner Detection](data/features/features_8.png)

This implementation demonstrates feature detection; persistent feature matching and tracking across frames are not implemented.

## Technologies

- C++17
- OpenCV 4
- Camera calibration and distortion estimation
- Perspective-n-Point pose estimation
- 3D-to-2D projection
- Harris corner detection

## Project Structure

```text
camera-calibration-ar/
├── src/               # C++ implementations
├── include/           # Header files
├── data/
│   ├── calibration/   # Camera calibration YAML
│   ├── snapshots/     # Checkerboard calibration images
│   ├── position/      # Camera pose and augmented reality images
│   └── features/      # Harris corner detection images
├── docs/
│   └── project-report.md
├── Makefile
└── README.md
```


## Build and Execution

### Requirements

- C++17-compatible compiler
- GNU Make
- OpenCV 4.x

The project was successfully built on an Apple Silicon Mac using Clang and OpenCV 4.14.

### Install OpenCV (macOS / Homebrew)

```bash
brew install opencv@4
```

> Note: The Homebrew `opencv@4` formula is deprecated and may become unavailable in the future.

### Build

Clone the repository and navigate to its root directory.

```bash
make
```

The Makefile produces three executables in `bin/`:

- `detect_corners` — Checkerboard detection and camera calibration
- `pose_ar` — Camera pose estimation and augmented reality
- `robust_features` — Harris corner detection

### Run

From the project root:

```bash
./bin/detect_corners
./bin/pose_ar
./bin/robust_features
```

These programs use a webcam. The calibration and augmented reality applications require a 7 × 6 inner-corner checkerboard target.

The original camera parameters are stored in `data/calibration/calibration_result.yml`. Recalibration may be necessary when changing cameras or capture settings.

### Clean

```bash
make clean
```

On Apple Silicon, the default Makefile configuration expects OpenCV 4 at `/opt/homebrew/opt/opencv@4`. Set `OPENCV_PREFIX` when using another installation location.


## Documentation

See the [original project report](docs/project-report.md) for the full experimental methodology, screenshots, results, and reflections.

## Author

**Ruben A. Salido**

M.S. Computer Science — Northeastern University

[GitHub Profile](https://github.com/ElGavaOriginal)