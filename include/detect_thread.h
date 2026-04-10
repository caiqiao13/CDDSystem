#ifndef DETECT_THREAD
#define DETECT_THREAD

#include <QString>
#include <QThread>
#include <main_window_c.h>
#include <QImage>
#include <atomic>
#include "db_manager.h"

class DetectThreadBase : public QThread {
	Q_OBJECT

protected:
	MainWindow* main_window;
	bool is_label;

signals:
	void frameReady(QImage image);
	void detectionLog(QString log);
	void recordReady(DetectionRecord record);

public:
	static std::atomic<bool> STOP;
	static std::atomic<int> ID;
	static std::atomic<QThread*> cur_thread;
	static void stop();


	DetectThreadBase(MainWindow& mw, bool label);
	virtual void do_detect() = 0;
	virtual int get_detect_id() = 0;
	virtual void run();
	virtual void before_running();
	virtual void after_running();
	virtual void clear();
	std::vector<Detection> run_detection(cv::Mat& frame);

};

class ImageDetectThread : public DetectThreadBase {
private:
	QString file;
public:
	const static int ID;
	ImageDetectThread(MainWindow& mw, bool label, const QString& file);
	void do_detect();
	int get_detect_id();
};

class VideoDetectThread : public DetectThreadBase {
private:
	const QString file;

public:
	const static int ID;
	VideoDetectThread(MainWindow& mw, bool label, const QString& file);
	void do_detect();
	int get_detect_id();
};

class CameraDetectThread : public DetectThreadBase {

public:
	const static int ID;
	CameraDetectThread(MainWindow& mw, bool label);
	void do_detect();
	int get_detect_id();
};

class BatchImageDetectThread : public DetectThreadBase {
private:
	QString folder;
public:
	const static int ID;
	BatchImageDetectThread(MainWindow& mw, bool label, const QString& folder);
	void do_detect();
	int get_detect_id();
};

// ...


class WarmUpThread : public QThread {
private:
	float iou_thres;
	float conf_thres;
	QString path;
	MainWindow* main_window;
	cv::Size input_shape;

public:
	WarmUpThread(MainWindow& mw, const QString& path, float iou_thres, float conf_thres, const cv::Size& input_shape);
	virtual void run();
};

#endif // DETECT_THREAD
