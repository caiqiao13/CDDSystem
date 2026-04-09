#ifndef INFERENCE_H
#define INFERENCE_H

// YOLO 官方模型example，进行了一些修改

// Cpp native
#include <fstream>
#include <vector>
#include <string>
#include <random>

// OpenCV / DNN / Inference
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

struct Detection
{
    int class_id{0};
    float confidence{ 0.0 };
    std::string className{};
    cv::Scalar color{};
    cv::Rect box{};
};

class Inference
{
public:
    const int CPU = 0;
	const int GPU = 1;

    Inference();
    
    void doPredict(cv::Mat frame, bool label_img = false);
    std::vector<Detection> doPredictWithResults(cv::Mat frame, bool label_img = false);
    bool loadOnnxNetwork(const std::string& onnxModelPath, const cv::Size& modelInputShape, const bool& runWithCuda);
    float modelScoreThreshold{ 0.45 };
    float modelNMSThreshold{ 0.50 };
    bool letterBoxForSquare = true;
    void loadClassesFromFile();

    std::string model_name;
    std::vector<std::string> classes{ "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush" };


private:
    bool cudaEnabled{};
    float modelConfidenceThreshold {0.25};

    std::string modelPath{};
    std::vector<Detection> runInference(const cv::Mat& input);

    cv::dnn::Net net;
    cv::Size2f modelShape{};
    cv::Mat formatToSquare(const cv::Mat& source);
};

#endif // INFERENCE_H
