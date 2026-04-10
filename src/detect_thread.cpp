#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <detect_thread.h>
#include <QMessageBox>
#include <QThread>
#include <opencv2/opencv.hpp>

#include "commons.h"
#include "inference.h"
#include "db_manager.h"
#include <QDateTime>
#include <QMetaObject>

std::atomic<bool> DetectThreadBase::STOP{false};
std::atomic<int> DetectThreadBase::ID{0};
std::atomic<QThread*> DetectThreadBase::cur_thread{nullptr};

DetectThreadBase::DetectThreadBase(MainWindow& mw, bool label) : main_window(&mw), is_label(label)
{
}


void DetectThreadBase::stop()
{
	DetectThreadBase::STOP = true;
}

void DetectThreadBase::run() {
	this->before_running();
	this->do_detect();
	this->after_running();
}


void DetectThreadBase::before_running()
{
	DetectThreadBase::ID = this->get_detect_id();
	DetectThreadBase::STOP = false;
	DetectThreadBase::cur_thread = this;
}

void DetectThreadBase::after_running()
{
	DetectThreadBase::ID = 0;                // 重置ID未0，表示没有线程在检测
	DetectThreadBase::STOP = true;		     // 停止检测
	DetectThreadBase::cur_thread = nullptr;  
}

void DetectThreadBase::clear()
{
}

std::vector<Detection> DetectThreadBase::run_detection(cv::Mat& frame) {
    if (this->main_window->currentDetectorType >= 0 && this->main_window->currentDetectorType <= 3) {
        std::vector<Detection> results;
        if (!this->main_window->opencvDetector) return results;
        
        auto opencv_results = this->main_window->opencvDetector->detect(frame);
        for (const auto& res : opencv_results) {
            Detection d;
            d.class_id = 0;
            d.confidence = res.confidence;
            d.className = res.className;
            d.color = res.color;
            d.box = res.box;
            results.push_back(d);

            cv::rectangle(frame, d.box, d.color, 2);
            if (this->is_label) {
                std::string confStr = std::to_string(d.confidence);
                if (confStr.length() > 4) confStr = confStr.substr(0, 4);
                std::string classString = d.className + ' ' + confStr;
                cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 1, 2, 0);
                cv::Rect textBox(d.box.x, d.box.y - 40, textSize.width + 10, textSize.height + 20);
                cv::rectangle(frame, textBox, d.color, cv::FILLED);
                cv::putText(frame, classString, cv::Point(d.box.x + 5, d.box.y - 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 2, 0);
            }
        }
        return results;
    } else {
        if (!this->main_window->inference) return {};
        return this->main_window->inference->doPredictWithResults(frame, this->is_label);
    }
}

const int ImageDetectThread::ID = 1;
ImageDetectThread::ImageDetectThread(MainWindow& mw, bool label, const QString& file) :DetectThreadBase(mw, label), file(file) {

}
int ImageDetectThread::get_detect_id()
{
	return ImageDetectThread::ID;
}
void ImageDetectThread::do_detect() {
	if (!this->main_window->check_model()) {
		emit this->detectionLog("Error: 未检测到模型");
		return;
	}
	QFile imgFile(this->file);
	if (!imgFile.open(QIODevice::ReadOnly)) {
		emit this->detectionLog("Error: 打开图片文件失败 " + this->file);
		return;
	}
	QByteArray imgData = imgFile.readAll();
	std::vector<char> buffer(imgData.begin(), imgData.end());
	cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);

	if (frame.empty()) {
		emit this->detectionLog("Error: 读取图片失败 " + this->file);
		return;
	}
	std::vector<Detection> results = this->run_detection(frame);
	QImage image = commons::cvMatToQImage(frame);
	emit this->frameReady(image);
	
	for (const auto& det : results) {
		DetectionRecord record;
		record.image_path = this->file;
		record.defect_type = QString::fromStdString(det.className);
		record.confidence = det.confidence;
		record.bounding_box = QString("%1,%2,%3,%4").arg(det.box.x).arg(det.box.y).arg(det.box.width).arg(det.box.height);
		record.detection_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
		emit this->recordReady(record);
	}
	
	emit this->detectionLog("图片检测完成: " + this->file);
}


const int VideoDetectThread::ID = 2;
VideoDetectThread::VideoDetectThread(MainWindow& mw, bool label, const QString& file) :DetectThreadBase(mw, label), file(file) {

}
int VideoDetectThread::get_detect_id()
{
	return VideoDetectThread::ID;
}
void VideoDetectThread::do_detect() {
	emit this->main_window->start_loading_movie_signal();
	cv::VideoCapture cap;
	cap.open(this->file.toLocal8Bit().constData());
	emit this->main_window->stop_loading_movie_signal();

	if (!cap.isOpened()) {
		emit this->detectionLog("Error: 打开视频失败 " + this->file);
		return;
	}

	int idx = 0;
	emit this->detectionLog("开始视频检测: " + this->file);
	while (!DetectThreadBase::STOP)
	{
		idx++;
		if (idx % 2 == 0) {
			idx = 0;
			continue;
		}  
		// 获取新的一帧;
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()) {
			emit this->detectionLog("视频播放结束: " + this->file);
			break;
		}
		// 显示新的帧;
		std::vector<Detection> results = this->run_detection(frame);
		QImage image = commons::cvMatToQImage(frame);
		emit this->frameReady(image);

		for (const auto& det : results) {
			DetectionRecord record;
			record.image_path = this->file;
			record.defect_type = QString::fromStdString(det.className);
			record.confidence = det.confidence;
			record.bounding_box = QString("%1,%2,%3,%4").arg(det.box.x).arg(det.box.y).arg(det.box.width).arg(det.box.height);
			record.detection_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			emit this->recordReady(record);
		}
	}
	cap.release();
}

const int CameraDetectThread::ID = 3;
CameraDetectThread::CameraDetectThread(MainWindow& mw, bool label) : DetectThreadBase(mw, label) {

}

int CameraDetectThread::get_detect_id()
{
	return CameraDetectThread::ID;
}


void CameraDetectThread::do_detect() {

	emit this->main_window->start_loading_movie_signal();
	cv::VideoCapture cap;
	cap.open(0);
	emit this->main_window->stop_loading_movie_signal();

	if (!cap.isOpened()) {
		emit this->detectionLog("Error: 打开摄像头失败");
		return;
	}
	emit this->detectionLog("开始摄像头检测");
	int idx = 0;

	while (!DetectThreadBase::STOP)
	{
		idx++;
		if (idx % 2 == 0) {
			idx = 0;
			continue;
		}
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()) {
			emit this->detectionLog("Error: 摄像头帧为空");
			break;
		}
		std::vector<Detection> results = this->run_detection(frame);
		QImage image = commons::cvMatToQImage(frame);
		emit this->frameReady(image);

		for (const auto& det : results) {
			DetectionRecord record;
			record.image_path = "Camera";
			record.defect_type = QString::fromStdString(det.className);
			record.confidence = det.confidence;
			record.bounding_box = QString("%1,%2,%3,%4").arg(det.box.x).arg(det.box.y).arg(det.box.width).arg(det.box.height);
			record.detection_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			emit this->recordReady(record);
		}
	}
	cap.release();
}

const int BatchImageDetectThread::ID = 4;
BatchImageDetectThread::BatchImageDetectThread(MainWindow& mw, bool label, const QString& folder)
	: DetectThreadBase(mw, label), folder(folder)
{
}

int BatchImageDetectThread::get_detect_id()
{
	return BatchImageDetectThread::ID;
}

void BatchImageDetectThread::do_detect()
{
	QDir dir(this->folder);
	if (!dir.exists()) {
		emit this->detectionLog("Error: 文件夹不存在 " + this->folder);
		return;
	}
	QStringList filters;
	filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
	QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
	if (files.isEmpty()) {
		emit this->detectionLog("Error: 文件夹内无支持的图片 (*.png *.jpg *.jpeg *.bmp)");
		return;
	}
	if (!this->main_window->check_model()) {
		emit this->detectionLog("Error: 未检测到模型");
		return;
	}
	emit this->detectionLog(QString("批量检测开始，共 %1 张").arg(files.size()));
	int done = 0;
	for (const QString& name : files) {
		if (DetectThreadBase::STOP) {
			emit this->detectionLog("批量检测已停止");
			break;
		}
		QString path = dir.filePath(name);
		QFile imgFile(path);
		if (!imgFile.open(QIODevice::ReadOnly)) {
			emit this->detectionLog("Error: 打开图片失败 " + path);
			continue;
		}
		QByteArray imgData = imgFile.readAll();
		std::vector<char> buffer(imgData.begin(), imgData.end());
		cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
		if (frame.empty()) {
			emit this->detectionLog("Error: 解码图片失败 " + path);
			continue;
		}
		std::vector<Detection> results = this->run_detection(frame);
		QImage image = commons::cvMatToQImage(frame);
		emit this->frameReady(image);
		for (const auto& det : results) {
			DetectionRecord record;
			record.image_path = path;
			record.defect_type = QString::fromStdString(det.className);
			record.confidence = det.confidence;
			record.bounding_box = QString("%1,%2,%3,%4").arg(det.box.x).arg(det.box.y).arg(det.box.width).arg(det.box.height);
			record.detection_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			emit this->recordReady(record);
		}
		done++;
		emit this->detectionLog(QString("批量进度 %1/%2: %3").arg(done).arg(files.size()).arg(name));
	}
	emit this->detectionLog(QString("批量检测结束，已处理 %1 张").arg(done));
}

WarmUpThread::WarmUpThread(MainWindow& mw, const QString& path, float iou_thres, float conf_thres, const cv::Size& input_shape)
	:main_window(&mw), path(path), iou_thres(iou_thres), conf_thres(conf_thres), input_shape(input_shape)
{
}

void WarmUpThread::run()
{
	auto& inference = this->main_window->inference;
	if (inference == nullptr)
	{
		inference.reset(new Inference());
	}
	emit this->main_window->start_loading_movie_signal();
	if (!inference->loadOnnxNetwork(this->path.toStdString(), this->input_shape, true)) {
		this->main_window->inference.reset();
		QMetaObject::invokeMethod(this->main_window, "append_log", Qt::QueuedConnection,
			Q_ARG(QString, "Error: ONNX 模型加载失败，请检查权重文件"));
		emit this->main_window->stop_loading_movie_signal();
		return;
	}
	inference->modelNMSThreshold = this->iou_thres;
	inference->modelScoreThreshold = this->conf_thres;
	cv::Mat zero = cv::Mat::zeros(this->input_shape, CV_8UC3);
	inference->doPredict(zero);
	emit this->main_window->stop_loading_movie_signal();
}
