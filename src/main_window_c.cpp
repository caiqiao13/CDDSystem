#pragma execution_character_set("utf-8")
#include <iostream>
#include <QPushButton>
#include <QApplication>
#include <QString>
#include <QMovie>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <vector>
#include <QStandardItemModel>
#include "main_window_c.h"
#include "commons.h"
#include "detect_thread.h"
#include "db_manager.h"
#include "db_config_dialog.h"
#include <QSettings>

MainWindow::MainWindow()
{
    ui.setupUi(this);
    this->connect_slots();

    // Init DB
    QSettings settings("config.ini", QSettings::IniFormat);
    QString host = settings.value("Database/Host", "127.0.0.1").toString();
    int port = settings.value("Database/Port", "3306").toInt();
    QString dbName = settings.value("Database/Name", "defect_db").toString();
    QString user = settings.value("Database/User", "root").toString();
    QString password = settings.value("Database/Password", "root").toString();

    if (DBManager::getInstance().connectToDatabase(host, dbName, user, password, port)) {
        this->append_log("数据库连接成功");
    } else {
        this->append_log("Error: 数据库连接失败");
    }

    // init spinbox
    this->ui.iou_spinbox->setRange(0, 1);
    this->ui.iou_spinbox->setSingleStep(0.01);
    this->ui.iou_slider->setTracking(false);
    this->ui.conf_spinbox->setRange(0, 1);
    this->ui.conf_slider->setTracking(false);
    this->ui.conf_spinbox->setSingleStep(0.01);

    // init slider
    this->ui.iou_slider->setRange(0, 100);
    this->ui.iou_slider->setSingleStep(1);
    this->ui.conf_slider->setRange(0, 100);
    this->ui.conf_slider->setSingleStep(1);

	// 固定窗口大小
    this->setWindowTitle("陶瓷表面缺陷检测系统");
    this->setWindowIcon(QIcon(":/icon/JYD.ico"));
    this->setFixedSize(this->width(), this->height());

	// 设置默认IOU和CONF
	this->reset_iou_slot();
	this->reset_conf_slot();
    
    
	this->refresh_model_list();   // 刷新模型列表
	emit this->ui.model_select->activated(0);  // 激活第一个模型
    this->show();

}

MainWindow::~MainWindow()
{
}

void MainWindow::show_img(QString& path) {
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "Error", path + "图片加载失败");
    }
    QPixmap pixmap(path);
    this->show_img(pixmap);
    //ui.show_detect->setPixmap(pixmap.scaled(ui.show_detect->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::show_img(QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to load image");
        return;
    }
    int width = ui.show_detect->width();
    int height = ui.show_detect->height();
    float rate = 0.8f;
    ui.show_detect->setAlignment(Qt::AlignCenter);
    ui.show_detect->setPixmap(pixmap.scaled(int(width * rate), int(height * rate), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::show_img(QPixmap&& pixmap) {
    this->show_img(pixmap);
}

bool prepare_thread(int thread_id, MainWindow* mw) {
    if (DetectThreadBase::ID == thread_id) {
		// ...
        return true;
    }
    else if (DetectThreadBase::ID != 0) {
        // ...
        DetectThreadBase::stop();
        if (mw->global_threading) {
            mw->global_threading->wait();
        }
    }
    return false;
}
// slots
void MainWindow::cammera_slot()
{
    if (prepare_thread(CameraDetectThread::ID, this)) {
        return;
    }
    if (!this->check_model()) {  // 模型未通过检查
        return;
    }
    CameraDetectThread* dt = new CameraDetectThread(*this, this->ui.islabel->isChecked());
    QObject::connect(dt, &DetectThreadBase::frameReady, this, &MainWindow::onFrameReady);
    QObject::connect(dt, &DetectThreadBase::detectionLog, this, &MainWindow::onDetectionLog);
    QObject::connect(dt, &DetectThreadBase::recordReady, this, &MainWindow::onRecordReady);
    this->global_threading.reset(dt);
    dt->start();
}

void MainWindow::image_slot()
{
    if (prepare_thread(ImageDetectThread::ID, this)) {
        return;
    }
    if (!this->check_model()) {  // 模型未通过检查
        return;
    }
    // 选择图片文件
    QString file = QFileDialog::getOpenFileName(this, "Open File", ".", "Images(*.png *.jpg)");
    if (file == "") {
        return;
    }
    // ...
    ImageDetectThread* dt = new ImageDetectThread(*this, this->ui.islabel->isChecked(), file);
    QObject::connect(dt, &DetectThreadBase::frameReady, this, &MainWindow::onFrameReady);
    QObject::connect(dt, &DetectThreadBase::detectionLog, this, &MainWindow::onDetectionLog);
    QObject::connect(dt, &DetectThreadBase::recordReady, this, &MainWindow::onRecordReady);
    this->global_threading.reset(dt);
    dt->start();
}

void MainWindow::video_slot()
{
    if (prepare_thread(VideoDetectThread::ID, this)) {
        return;
    }
    if (!this->check_model()) {  // 模型未通过检查
        return;
    }
    QString file = QFileDialog::getOpenFileName(this, "Open File", ".", "Videos(*.mp4)");
    if (file == "") {
        return;
    }
    VideoDetectThread* dt = new VideoDetectThread(*this, this->ui.islabel->isChecked(), file);
    QObject::connect(dt, &DetectThreadBase::frameReady, this, &MainWindow::onFrameReady);
    QObject::connect(dt, &DetectThreadBase::detectionLog, this, &MainWindow::onDetectionLog);
    QObject::connect(dt, &DetectThreadBase::recordReady, this, &MainWindow::onRecordReady);
    this->global_threading.reset(dt);
    dt->start();
}

void MainWindow::iou_spinbox_slot(double value)
{
    this->ui.iou_slider->blockSignals(true);
    this->ui.iou_slider->setValue(int(value*100));
    this->ui.iou_slider->blockSignals(false);
}

void MainWindow::conf_spinbox_slot(double value)
{
    this->ui.conf_slider->blockSignals(true);
    this->ui.conf_slider->setValue(int(value * 100));
    this->ui.conf_slider->blockSignals(false);

}

void MainWindow::iou_slider_slot(int value) {
    this->ui.iou_spinbox->blockSignals(true);
    this->ui.iou_spinbox->setValue(value * 1.0 / 100);
    this->ui.iou_spinbox->blockSignals(false);
}

void MainWindow::conf_slider_slot(int value) {
    this->ui.conf_spinbox->blockSignals(true);
    this->ui.conf_spinbox->setValue(value * 1.0 / 100);
    this->ui.conf_spinbox->blockSignals(false);
}

void MainWindow::reset_iou_slot()
{
    this->iou_slider_slot(50);
    this->iou_spinbox_slot(0.5);
}

void MainWindow::reset_conf_slot()
{
    this->conf_slider_slot(45);
    this->conf_spinbox_slot(0.45);
}

void MainWindow::model_select_slot(int index)
{
    if (index == this->current_weight_index or this->ui.model_select->count() == 0) {
		return;
    }
	this->current_weight_index = index;
    QString file = this->ui.model_select->itemText(index);

	// ...
	if (index >= 1 && index <= 4) {
		// OpenCV 原生模型
		this->currentDetectorType = index - 1;
		
		// 初始化 OpenCV 模型
		if (!this->opencvDetector) {
			this->opencvDetector.reset(new OpenCVDetector());
		}
		
		OpenCVDetector::DetectorType type = static_cast<OpenCVDetector::DetectorType>(this->currentDetectorType);
		if (this->opencvDetector->initialize(type)) {
			this->append_log("已选择 " + QString::fromStdString(this->opencvDetector->getDetectorName()));
		} else {
			this->append_log("Error: 初始化 " + QString::fromStdString(this->opencvDetector->getDetectorName()) + "失败");
		}
		return;
	}
	
	// ...
	if (index == 0 || index == 6 || file.isEmpty() || file.startsWith("===")) {
		this->current_weight_index = -1;
		return;
	}
	
	// YOLO ONNX 模型
	this->currentDetectorType = -1; // Reset OpenCV detector type
	float iou = this->ui.iou_spinbox->value();
	float conf = this->ui.conf_spinbox->value();
    // 加载模型

    this->global_threading.reset(new WarmUpThread(*this, "weights/" + file, iou, conf, cv::Size(640, 640)));
    this->global_threading->start();
}

void MainWindow::import_model_button_slot()
{
	QString file = QFileDialog::getOpenFileName(this, "Open File", ".", "Models(*.onnx *.txt)");
	if (file == "") {
		return;
	}
	bool copy_success = commons::copy_weights(file, "weights");
    if (copy_success) {
        this->refresh_model_list();
        this->append_log("加载成功");
		// 加载模型

        if (file.endsWith(".onnx")) {
            std::string file_name = commons::get_file_name(file.toStdString());;
			int index = this->ui.model_select->findText(QString::fromStdString(file_name));
			if (index != -1) {
				this->current_weight_index = -1; // 加载模型
				emit this->ui.model_select->activated(index);
			}
        }
    }
}

void MainWindow::config_db_slot()
{
    DbConfigDialog dialog(this);
    dialog.exec();
}

void MainWindow::start_loading_movie_slot()
{
    if (this->movie == nullptr) {
        this->movie.reset(new QMovie(":/gif/loading.gif"));
        QRect rect = this->ui.show_detect->geometry();
        int bw = rect.width();
        int bh = rect.height();
        double scale = std::min(bw, bh) * 0.4;
        this->movie->setScaledSize(QSize(scale, scale));
    }

    if (this->ui.show_detect->movie() == nullptr) {
		this->ui.show_detect->setMovie(this->movie.get());
		this->ui.show_detect->setAlignment(Qt::AlignCenter);
    }
    auto m = this->ui.show_detect->movie();
    if (m != nullptr) {
        m->start();
    }
    this->disable_all_button();
}

void MainWindow::stop_loading_movie_slot()
{
	if (this->ui.show_detect->movie() != nullptr) {
		this->ui.show_detect->movie()->stop();
        this->ui.show_detect->clear();
	}
	this->enable_all_button();
}

void MainWindow::islabel_slot(bool value)
{
    this->ui.islabel->setChecked(value);
}

void MainWindow::connect_slots()
{
    QObject::connect(ui.cammera_button, SIGNAL(clicked()), this, SLOT(cammera_slot()));
    QObject::connect(ui.image_button, SIGNAL(clicked()), this, SLOT(image_slot()));
    QObject::connect(ui.video_button, SIGNAL(clicked()), this, SLOT(video_slot()));

    QObject::connect(ui.iou_spinbox, SIGNAL(valueChanged(double)), this, SLOT(iou_spinbox_slot(double)));
    QObject::connect(ui.conf_spinbox, SIGNAL(valueChanged(double)), this, SLOT(conf_spinbox_slot(double)));

    QObject::connect(ui.iou_slider, SIGNAL(valueChanged(int)), this, SLOT(iou_slider_slot(int)));
    QObject::connect(ui.conf_slider, SIGNAL(valueChanged(int)), this, SLOT(conf_slider_slot(int)));

    QObject::connect(ui.reset_iou_button, SIGNAL(clicked()), this, SLOT(reset_iou_slot()));
    QObject::connect(ui.reset_conf_button, SIGNAL(clicked()), this, SLOT(reset_conf_slot()));
	QObject::connect(ui.model_select, SIGNAL(activated(int)), this, SLOT(model_select_slot(int)));
	QObject::connect(ui.import_model_button, SIGNAL(clicked()), this, SLOT(import_model_button_slot()));
    QObject::connect(ui.config_db_button, SIGNAL(clicked()), this, SLOT(config_db_slot()));
    
	QObject::connect(this, SIGNAL(start_loading_movie_signal()), this, SLOT(start_loading_movie_slot()));
	QObject::connect(this, SIGNAL(stop_loading_movie_signal()), this, SLOT(stop_loading_movie_slot()));
}

void MainWindow::refresh_model_list()
{
	// 加载模型
	QDir dir("weights");
	QStringList filters;
	filters << "*.onnx";
	QStringList files = dir.entryList(filters);
	
	ui.model_select->clear();
	
	// 加载 OpenCV 原生模型
	int sep1Index = ui.model_select->count();
	ui.model_select->addItem("=== OpenCV 原生模型 ===");
	ui.model_select->addItem("Haar Face (人脸)");
	ui.model_select->addItem("Haar Eye (眼睛)");
	ui.model_select->addItem("Haar Smile (微笑)");
	ui.model_select->addItem("HOG Person (行人)");
	
	// 加载模型
	if (!files.isEmpty()) {
		ui.model_select->addItem("");  // ...
		ui.model_select->addItem("=== YOLO ONNX 模型 ===");
		ui.model_select->addItems(files);
	}
	
	// ...
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.model_select->model());
	if (model) {
		model->item(sep1Index)->setEnabled(false);
		if (!files.isEmpty()) {
			model->item(sep1Index + 6)->setEnabled(false);  // ...
		}
	}
}

void MainWindow::disable_all_button()
{
	this->ui.cammera_button->setDisabled(true);
	this->ui.image_button->setDisabled(true);
	this->ui.video_button->setDisabled(true);
	this->ui.model_select->setDisabled(true);
	this->ui.import_model_button->setDisabled(true);

}

void MainWindow::enable_all_button()
{
	this->ui.cammera_button->setDisabled(false);
	this->ui.image_button->setDisabled(false);
	this->ui.video_button->setDisabled(false);
	this->ui.model_select->setDisabled(false);
	this->ui.import_model_button->setDisabled(false);
}

bool MainWindow::check_model()
{
    if (this->currentDetectorType >= 0 && this->currentDetectorType <= 3) {
        if (this->opencvDetector != nullptr) return true;
        return false;
    }

    // 加载模型
    if (this->inference == nullptr and this->ui.model_select->count() == 0) {
        QMessageBox::warning(this, "Error", QString("未找到模型"));
        return false;
    }

	// 加载模型
    std::string base_name = commons::get_base_name(this->ui.model_select->currentText().toStdString());
    std::string txt_name = base_name + ".txt";
    std::string txt_path = "weights/" + txt_name;
    if (!QFile::exists(QString::fromStdString(txt_path))) {
        QMessageBox::warning(this, "Error", QString::fromStdString(txt_name) + "不存在, 请导入...");
        return false;
    }
    if (this->inference != nullptr) {
        this->inference->loadClassesFromFile();
    }
    return true;
}

void MainWindow::load_model_from_model_list()
{
	int index = this->ui.model_select->currentIndex();
	this->model_select_slot(index);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (this->global_threading != nullptr) {
		DetectThreadBase::stop();
        this->global_threading->wait();
	}
}

void MainWindow::append_log(QString text)
{
    QString timeStr = QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss] ");
    QString fullText = timeStr + text;
    ui.output->append(fullText);

    QFile logFile("system_log.txt");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << fullText << "\n";
        logFile.close();
    }
}

void MainWindow::onFrameReady(QImage image)
{
    QPixmap pixmap = QPixmap::fromImage(image);
    this->show_img(pixmap);
}

void MainWindow::onDetectionLog(QString log)
{
    this->append_log(log);
}

void MainWindow::onRecordReady(DetectionRecord record)
{
    DBManager::getInstance().insertRecord(record);
}
