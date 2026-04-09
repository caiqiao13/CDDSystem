
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QDir>
#include <QMessageBox>

#include "opencv2/opencv.hpp"
#include "commons.h"
namespace commons {


    // 将Mat转化位QImage
    QImage  cvMatToQImage(const cv::Mat& inMat)
    {
        switch (inMat.type())
        {
            // 8-bit, 4 channel
        case CV_8UC4:
        {
            QImage image(inMat.data,
                inMat.cols, inMat.rows,
                static_cast<int>(inMat.step),
                QImage::Format_ARGB32);

            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
            QImage image(inMat.data,
                inMat.cols, inMat.rows,
                static_cast<int>(inMat.step),
                QImage::Format_RGB888);

            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image(inMat.data,
                inMat.cols, inMat.rows,
                static_cast<int>(inMat.step),
                QImage::Format_Grayscale8);//Format_Alpha8 and Format_Grayscale8 were added in Qt 5.5
#else//这里还有一种写法，最后给出
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if (sColorTable.isEmpty())
            {
                sColorTable.resize(256);

                for (int i = 0; i < 256; ++i)
                {
                    sColorTable[i] = qRgb(i, i, i);
                }
            }

            QImage image(inMat.data,
                inMat.cols, inMat.rows,
                static_cast<int>(inMat.step),
                QImage::Format_Indexed8);

            image.setColorTable(sColorTable);
#endif

            return image;
        }

        default:
            qWarning() << "CVS::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
        }

        return QImage();
    }

    //将Mat转化为QPixmap
    QPixmap cvMatToQPixmap(const cv::Mat& inMat)
    {
        return QPixmap::fromImage(cvMatToQImage(inMat));
    }

    //将QImage转化为Mat
    cv::Mat QImageToCvMat(const QImage& inImage, bool inCloneImageData = true)
    {
        switch (inImage.format())
        {
            // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
            cv::Mat  mat(inImage.height(), inImage.width(),
                CV_8UC4,
                const_cast<uchar*>(inImage.bits()),
                static_cast<size_t>(inImage.bytesPerLine())
            );

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
        {
            if (!inCloneImageData)
            {
                qWarning() << "CVS::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
            }

            QImage   swapped = inImage;

            if (inImage.format() == QImage::Format_RGB32)
            {
                swapped = swapped.convertToFormat(QImage::Format_RGB888);
            }

            swapped = swapped.rgbSwapped();

            return cv::Mat(swapped.height(), swapped.width(),
                CV_8UC3,
                const_cast<uchar*>(swapped.bits()),
                static_cast<size_t>(swapped.bytesPerLine())
            ).clone();
        }

        // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
            cv::Mat  mat(inImage.height(), inImage.width(),
                CV_8UC1,
                const_cast<uchar*>(inImage.bits()),
                static_cast<size_t>(inImage.bytesPerLine())
            );

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qWarning() << "CVS::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
            break;
        }

        return cv::Mat();
    }

    //将QPixmap转化为Mat
    cv::Mat QPixmapToCvMat(const QPixmap& inPixmap, bool inCloneImageData = true)
    {
        return QImageToCvMat(inPixmap.toImage(), inCloneImageData);
    }

    void mkdir(const QString& path)
    {
		QDir targetDir(path);
        QDir dir(".");
        if (!targetDir.exists())
        {
            dir.mkdir(path);
        }
    }

	bool copy_weights(const QString& weight, const QString& dir)
	{
        mkdir(dir);
		QFile file(weight);
		if (!file.exists())
		{
			qDebug() << "file not exists";
			return false;
		}
		QFileInfo info(file);
		QString name = info.fileName();
		QString new_path = dir + "/" + name;
		if (QFile::exists(new_path))
		{
			QMessageBox msgBox;
			msgBox.setBaseSize(300, 100);
			msgBox.setText("文件已存在，是否覆盖？");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int ret = msgBox.exec();
            if (ret == QMessageBox::Yes) {
                QFile::remove(new_path);
                QFile::copy(weight, new_path);
                return true;
            }
            else {
                return false;
            }
		}
        else
        {
            QFile::copy(weight, new_path);
            return true;
        }
        return false;
	}

    std::string get_base_name(const std::string& path)
    {
        std::string modelName = path.substr(path.find_last_of("/\\") + 1);
        std::string baseName = modelName.substr(0, modelName.find_last_of("."));
        return baseName;
    }

	std::string get_file_name(const std::string& path)
	{
		std::string modelName = path.substr(path.find_last_of("/\\") + 1);
		return modelName;
	}

}