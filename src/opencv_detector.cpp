﻿﻿#include "opencv_detector.h"
#include <iostream>

OpenCVDetector::OpenCVDetector() : currentType(YOLO_ONNX)
{
}

OpenCVDetector::~OpenCVDetector()
{
}

bool OpenCVDetector::initialize(DetectorType type)
{
    currentType = type;
    
    switch (type) {
        case HAAR_FACE:
        case HAAR_EYE:
        case HAAR_SMILE:
            return setupHaarCascade(type);
        case HOG_PERSON:
            return setupHOG();
        case YOLO_ONNX:
            return true;
        default:
            return false;
    }
}

bool OpenCVDetector::setupHaarCascade(DetectorType type)
{
    std::string cascadePath;
    
    try {
        switch (type) {
            case HAAR_FACE:
                cascadePath = cv::samples::findFile("haarcascades/haarcascade_frontalface_default.xml", false);
                if (cascadePath.empty() || !faceCascade.load(cascadePath)) {
                    std::cerr << "Error loading face cascade" << std::endl;
                    return false;
                }
                break;
            case HAAR_EYE:
                cascadePath = cv::samples::findFile("haarcascades/haarcascade_eye.xml", false);
                if (cascadePath.empty() || !eyeCascade.load(cascadePath)) {
                    std::cerr << "Error loading eye cascade" << std::endl;
                    return false;
                }
                break;
            case HAAR_SMILE:
                cascadePath = cv::samples::findFile("haarcascades/haarcascade_smile.xml", false);
                if (cascadePath.empty() || !smileCascade.load(cascadePath)) {
                    std::cerr << "Error loading smile cascade" << std::endl;
                    return false;
                }
                break;
            default:
                return false;
        }
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV Exception: " << e.what() << std::endl;
        return false;
    }
}

bool OpenCVDetector::setupHOG()
{
    try {
        hogDescriptor = cv::HOGDescriptor();
        hogDescriptor.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV Exception: " << e.what() << std::endl;
        return false;
    }
}

std::vector<DetectionResult> OpenCVDetector::detect(cv::Mat& frame)
{
    std::vector<DetectionResult> results;
    
    switch (currentType) {
        case HAAR_FACE: {
            std::vector<cv::Rect> faces;
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
            
            for (const auto& face : faces) {
                DetectionResult result;
                result.box = face;
                result.confidence = 0.9f;
                result.className = "Face";
                result.color = cv::Scalar(0, 255, 0);
                results.push_back(result);
            }
            break;
        }
        case HAAR_EYE: {
            std::vector<cv::Rect> eyes;
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            eyeCascade.detectMultiScale(gray, eyes, 1.1, 3, 0, cv::Size(20, 20));
            
            for (const auto& eye : eyes) {
                DetectionResult result;
                result.box = eye;
                result.confidence = 0.85f;
                result.className = "Eye";
                result.color = cv::Scalar(255, 0, 0);
                results.push_back(result);
            }
            break;
        }
        case HAAR_SMILE: {
            std::vector<cv::Rect> smiles;
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            smileCascade.detectMultiScale(gray, smiles, 1.1, 3, 0, cv::Size(30, 30));
            
            for (const auto& smile : smiles) {
                DetectionResult result;
                result.box = smile;
                result.confidence = 0.8f;
                result.className = "Smile";
                result.color = cv::Scalar(0, 0, 255);
                results.push_back(result);
            }
            break;
        }
        case HOG_PERSON: {
            std::vector<cv::Rect> persons;
            hogDescriptor.detectMultiScale(frame, persons, 0, cv::Size(), cv::Size(), 1.05, 2.0);
            
            for (const auto& person : persons) {
                DetectionResult result;
                result.box = person;
                result.confidence = 0.75f;
                result.className = "Person";
                result.color = cv::Scalar(0, 255, 255);
                results.push_back(result);
            }
            break;
        }
        case YOLO_ONNX:
            // YOLO ����� Inference �ദ��
            break;
        default:
            break;
    }
    
    return results;
}

std::string OpenCVDetector::getDetectorName() const
{
    switch (currentType) {
        case HAAR_FACE: return "Haar Face";
        case HAAR_EYE: return "Haar Eye";
        case HAAR_SMILE: return "Haar Smile";
        case HOG_PERSON: return "HOG Person";
        case YOLO_ONNX: return "YOLO ONNX";
        default: return "Unknown";
    }
}

OpenCVDetector::DetectorType OpenCVDetector::getCurrentType() const
{
    return currentType;
}
