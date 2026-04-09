#ifndef OPENCV_DETECTOR_H
#define OPENCV_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>

struct DetectionResult {
    cv::Rect box;
    float confidence;
    std::string className;
    cv::Scalar color;
};

class OpenCVDetector {
public:
    enum DetectorType {
        HAAR_FACE = 0,
        HAAR_EYE = 1,
        HAAR_SMILE = 2,
        HOG_PERSON = 3,
        YOLO_ONNX = 4
    };

    OpenCVDetector();
    ~OpenCVDetector();

    bool initialize(DetectorType type);
    std::vector<DetectionResult> detect(cv::Mat& frame);
    std::string getDetectorName() const;
    DetectorType getCurrentType() const;

private:
    DetectorType currentType;
    
    // Haar cascades
    cv::CascadeClassifier faceCascade;
    cv::CascadeClassifier eyeCascade;
    cv::CascadeClassifier smileCascade;
    
    // HOG descriptor
    cv::HOGDescriptor hogDescriptor;
    
    bool setupHaarCascade(DetectorType type);
    bool setupHOG();
};

#endif // OPENCV_DETECTOR_H
