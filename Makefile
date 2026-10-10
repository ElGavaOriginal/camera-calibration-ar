
CXX := clang++
CXXFLAGS := -std=c++17 -Wall -O2

# Homebrew OpenCV 4 installation (Apple Silicon)
OPENCV_PREFIX ?= /opt/homebrew/opt/opencv@4
INCLUDES := -I$(OPENCV_PREFIX)/include/opencv4 -I./include
LDFLAGS := -L$(OPENCV_PREFIX)/lib -Wl,-rpath,$(OPENCV_PREFIX)/lib
LDLIBS := -lopencv_core -lopencv_imgproc -lopencv_calib3d \
          -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs

BIN_DIR := bin

.PHONY: all clean

all: $(BIN_DIR)/detect_corners $(BIN_DIR)/pose_ar $(BIN_DIR)/robust_features

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/detect_corners: src/main.cpp src/target_detect.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BIN_DIR)/pose_ar: src/pose_ar.cpp src/target_detect.cpp src/tree_object.cpp src/sun_object.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BIN_DIR)/robust_features: src/robust_features.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -rf $(BIN_DIR)
