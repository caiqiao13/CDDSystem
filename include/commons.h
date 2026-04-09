#ifndef COMMONS
#define COMMONS
#include <QDebug>
#include <QImage>
#include <QPixmap>

#include "opencv2/opencv.hpp"
#include "commons.h"


namespace commons {
	// cv::Mat 转 QPixmap
	QPixmap cvMatToQPixmap(const cv::Mat& inMat);

	// cv::Mat 转 QImage
	QImage cvMatToQImage(const cv::Mat& inMat);

	cv::Mat QImageToCvMat(const QImage& inImage, bool inCloneImageData);

	cv::Mat QPixmapToCvMat(const QPixmap& inPixmap, bool inCloneImageData);

	void mkdir(const QString& path);

	bool copy_weights(const QString& weight, const QString& dir);

	std::string get_base_name(const std::string& path);

	std::string get_file_name(const std::string& path);
}

#endif // !COMMONS


