#include "gpswindow.h"
#include "ui_gpswindow.h"

gpsWindow::gpsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gpsWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/compass.png"));

    obj_devInfo = new Device_Info;
    obj_devInfo->m_requestPhonePermission();

    ui->tabWidget->setCurrentIndex(2);

    ui->tableWidget_GNACCURACY->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_QgpsPos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_gga->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_rmc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkpos1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkpos2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkagc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_gll->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_vtg->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkmpe1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    ui->tableWidget_GNACCURACY->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_rmc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkpos1->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkpos2->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkagc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkmpe1->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


    obj_gps = new GPS_Listener(this);
    connect(obj_gps,SIGNAL(si_Positions(gps_pos)),this, SLOT(sl_Positions(gps_pos)));
    connect(obj_gps,SIGNAL(si_gnAccuracy(double,double,double,double)),this, SLOT(sl_gnAccuracy(double,double,double,double)));
    connect(obj_gps,SIGNAL(si_error_msg(QString)),this, SLOT(sl_error_msg(QString)));
    connect(obj_gps,SIGNAL(si_ggaData(GgaData)),this, SLOT(sl_ggaData(GgaData)));
    connect(obj_gps,SIGNAL(si_rmcData(RmcData)),this, SLOT(sl_rmcData(RmcData)));
    connect(obj_gps,SIGNAL(si_mtkPos1(PmtkPos1)),this, SLOT(sl_mtkPos1(PmtkPos1)));
    connect(obj_gps,SIGNAL(si_mtkPos2(PmtkPos2)),this, SLOT(sl_mtkPos2(PmtkPos2)));
    connect(obj_gps,SIGNAL(si_mtkAgc(PmtkAgc)),this, SLOT(sl_mtkAgc(PmtkAgc)));
    connect(obj_gps,SIGNAL(si_gllData(GllData)),this, SLOT(sl_gllData(GllData)));
    connect(obj_gps,SIGNAL(si_vtgData(VtgData)),this, SLOT(sl_vtgData(VtgData)));
    connect(obj_gps,SIGNAL(si_skyPlot(GnssSystem, QVector<SkySat>)),this, SLOT(sl_skyPlot(GnssSystem, QVector<SkySat>)));
    connect(obj_gps,SIGNAL(si_mtkmpe1(PmtkMpe1)),this, SLOT(sl_mtkmpe1(PmtkMpe1)));


    obj_skyPlot = new GSV_Form(this);
    ui->skyPlotLayout->addWidget(obj_skyPlot);



    QTimer::singleShot(5000,this,SLOT(sl_DeviceInfo()));
}

gpsWindow::~gpsWindow()
{
    delete ui;
}

void gpsWindow::m_postData(const QString &serverUrl, QJsonObject payload)
{
    qDebug()<<__FUNCTION__;
    static QNetworkAccessManager manager;

    QUrl url(serverUrl); // e.g. http://localhost:8080/api/send_location
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json");

    QByteArray data =
        QJsonDocument(payload).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = manager.post(request, data);

    QObject::connect(reply, &QNetworkReply::finished,
                     [reply]() {
                         if (reply->error() == QNetworkReply::NoError) {
                             qDebug() << "POST success:"
                                      << reply->readAll();
                         } else {
                             qWarning() << "POST failed:"
                                        << reply->errorString();
                         }
                         reply->deleteLater();
                     });
}


void gpsWindow::sl_Positions(gps_pos gData)
{
    ui->tableWidget_QgpsPos->setItem(0, 1, new QTableWidgetItem(QString::number(gData.lat)));
    ui->tableWidget_QgpsPos->setItem(1, 1, new QTableWidgetItem(QString::number(gData.lon)));
    ui->tableWidget_QgpsPos->setItem(2, 1, new QTableWidgetItem(QString::number(gData.alt)));
    ui->tableWidget_QgpsPos->setItem(3, 1, new QTableWidgetItem(QString::number(gData.speed)));
    ui->tableWidget_QgpsPos->setItem(4, 1, new QTableWidgetItem(QString::number(gData.acc)));

    // JSON payload
    QJsonObject payload {
        {"utc", QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss")},
        {"lat", gData.lat},
        {"lon", gData.lon},
        {"alt", gData.alt},
        {"dev_id", (obj_devInfo?obj_devInfo->m_androidDeviceId():"ARISE1600")}
    };

    if(iCount%5 == 0)
    {
        m_postData("https://gpshttpserver.onrender.com/api/send_location",payload);
        qDebug ()<<"Trying...";
        if(iCount>4000)
            iCount = 1;
    }
    iCount++;

}

void gpsWindow::sl_DeviceInfo()
{
    diData.board = obj_devInfo->m_androidBoard();
    diData.product = obj_devInfo->m_androidProduct();
    diData.manufacturer = obj_devInfo->m_androidManufacturer();
    diData.model = obj_devInfo->m_androidModel();
    diData.android_id = obj_devInfo->m_androidId();
    diData.dev_id = obj_devInfo->m_androidDeviceId();
    diData.brand = obj_devInfo->m_getBrand();
    diData.device = obj_devInfo->m_getDevice();
    diData.hardware = obj_devInfo->m_getHardware();
    diData.android_ver = obj_devInfo->m_getAndroidVersion();
    diData.IMEI = obj_devInfo->m_getIMEI();

    qDebug() << obj_devInfo->m_printSimInfo();


    ui->label_AndroidID->setText(diData.android_id);
    ui->label_DevID->setText(diData.dev_id);
    ui->label_board->setText(diData.board);
    ui->label_manufacturer->setText(diData.manufacturer);
    ui->label_model->setText(diData.model);
    ui->label_product->setText(diData.product);

    ui->label_device->setText(diData.device);
    ui->label_brand->setText(diData.brand);
    ui->label_hw->setText(diData.hardware);
    ui->label_AndroidVer->setText(diData.android_ver);


    qDebug() << "Board:"        << diData.board;
    qDebug() << "Product:"      << diData.product;
    qDebug() << "Manufacturer:" << diData.manufacturer;
    qDebug() << "Model:"        << diData.model;
    qDebug() << "Android ID:"   << diData.android_id;
    qDebug() << "Device ID:"    << diData.dev_id;
    qDebug() << "Device :"    << diData.device;
    qDebug() << "Brand :"    << diData.brand;
    qDebug() << "Hardware :"    << diData.hardware;
    qDebug() << "IMEI :"    << diData.IMEI;



    // JSON payload
    QJsonObject payload {
        {"utc", QDateTime::currentDateTime().toString("yyyyMMdd hh:mm:ss")},
        {"board", diData.board},
        {"product", diData.product},
        {"manufacturer", diData.manufacturer},
        {"android_id", diData.android_id},
        {"model", diData.model},
        {"dev_id", diData.dev_id}
    };

    m_postData("https://gpshttpserver.onrender.com/api/send_devinfo",payload);
}

void gpsWindow::on_pushButton_Close_clicked()
{
    this->close();
}

void gpsWindow::sl_error_msg(QString msg)
{
    ui->plainTextEdit->appendPlainText(msg);
}

void gpsWindow::sl_gnAccuracy(double hAcc, double vAcc, double sAcc, double bAcc)
{
    ui->tableWidget_GNACCURACY->setItem(0, 1, new QTableWidgetItem(QString::number(hAcc)));
    ui->tableWidget_GNACCURACY->setItem(1, 1, new QTableWidgetItem(QString::number(vAcc)));
    ui->tableWidget_GNACCURACY->setItem(2, 1, new QTableWidgetItem(QString::number(sAcc)));
    ui->tableWidget_GNACCURACY->setItem(3, 1, new QTableWidgetItem(QString::number(bAcc)));

}

void gpsWindow::sl_ggaData(GgaData gga)
{
    ui->tableWidget_gga->setItem(0, 1, new QTableWidgetItem(gga.utcTime.toString("hh:mm:ss.zzz")));
    ui->tableWidget_gga->setItem(1, 1, new QTableWidgetItem(gga.talker));
    ui->tableWidget_gga->setItem(2, 1, new QTableWidgetItem(QString::number(gga.lat)));
    ui->tableWidget_gga->setItem(3, 1, new QTableWidgetItem(QString::number(gga.lon)));
    ui->tableWidget_gga->setItem(4, 1, new QTableWidgetItem(QString::number(gga.fixQuality)));
    ui->tableWidget_gga->setItem(5, 1, new QTableWidgetItem(QString::number(gga.satellites)));
    ui->tableWidget_gga->setItem(6, 1, new QTableWidgetItem(QString::number(gga.hdop)));
    ui->tableWidget_gga->setItem(7, 1, new QTableWidgetItem(QString::number(gga.altitude)));
    ui->tableWidget_gga->setItem(8, 1, new QTableWidgetItem(gga.valid?"TRUE":"FALSE"));


}

void gpsWindow::sl_rmcData(RmcData rmc)
{
    ui->tableWidget_rmc->setItem(0, 1, new QTableWidgetItem(rmc.utc.toString("yyyy/MM/dd hh:mm:ss.zzz")));
    ui->tableWidget_rmc->setItem(1, 1, new QTableWidgetItem(rmc.talker));
    ui->tableWidget_rmc->setItem(2, 1, new QTableWidgetItem(QString::number(rmc.latitude)));
    ui->tableWidget_rmc->setItem(3, 1, new QTableWidgetItem(QString::number(rmc.longitude)));
    ui->tableWidget_rmc->setItem(4, 1, new QTableWidgetItem(QString::number(rmc.speedKmh)));
    ui->tableWidget_rmc->setItem(5, 1, new QTableWidgetItem(QString::number(rmc.course)));
    ui->tableWidget_rmc->setItem(6, 1, new QTableWidgetItem(rmc.status));
    ui->tableWidget_rmc->setItem(7, 1, new QTableWidgetItem(rmc.mode));
    ui->tableWidget_rmc->setItem(8, 1, new QTableWidgetItem(rmc.valid?"TRUE":"FALSE"));


}

void gpsWindow::sl_mtkPos1(PmtkPos1 mtk_pos1)
{
    ui->tableWidget_mtkpos1->setItem(0, 1, new QTableWidgetItem(mtk_pos1.utc.toString("yyyy/MM/dd hh:mm:ss.zzz")));
    ui->tableWidget_mtkpos1->setItem(1, 1, new QTableWidgetItem(QString::number(mtk_pos1.lat)));
    ui->tableWidget_mtkpos1->setItem(2, 1, new QTableWidgetItem(QString::number(mtk_pos1.lon)));
    ui->tableWidget_mtkpos1->setItem(3, 1, new QTableWidgetItem(QString::number(mtk_pos1.alt)));
    ui->tableWidget_mtkpos1->setItem(4, 1, new QTableWidgetItem(QString::number(mtk_pos1.hVelocity)));
    ui->tableWidget_mtkpos1->setItem(5, 1, new QTableWidgetItem(QString::number(mtk_pos1.vVelocity)));
    ui->tableWidget_mtkpos1->setItem(6, 1, new QTableWidgetItem(mtk_pos1.valid?"TRUE":"FALSE"));

}

void gpsWindow::sl_mtkPos2(PmtkPos2 mtk_pos2)
{
    ui->tableWidget_mtkpos2->setItem(0, 1, new QTableWidgetItem(mtk_pos2.utc.toString("yyyy/MM/dd hh:mm:ss.zzz")));
    ui->tableWidget_mtkpos2->setItem(1, 1, new QTableWidgetItem(QString::number(mtk_pos2.lat)));
    ui->tableWidget_mtkpos2->setItem(2, 1, new QTableWidgetItem(QString::number(mtk_pos2.lon)));
    ui->tableWidget_mtkpos2->setItem(3, 1, new QTableWidgetItem(QString::number(mtk_pos2.alt)));
    ui->tableWidget_mtkpos2->setItem(4, 1, new QTableWidgetItem(QString::number(mtk_pos2.satellites)));

    ui->tableWidget_mtkpos2->setItem(5, 1, new QTableWidgetItem(QString::number(mtk_pos2.fixType)));
    ui->tableWidget_mtkpos2->setItem(6, 1, new QTableWidgetItem(QString::number(mtk_pos2.fwOrClockId)));
    ui->tableWidget_mtkpos2->setItem(7, 1, new QTableWidgetItem(QString::number(mtk_pos2.clockCounter)));
    ui->tableWidget_mtkpos2->setItem(8, 1, new QTableWidgetItem(mtk_pos2.valid?"TRUE":"FALSE"));

}

void gpsWindow::sl_mtkAgc(PmtkAgc mtk_agc)
{
    ui->tableWidget_mtkagc->setItem(0, 1, new QTableWidgetItem(mtk_agc.utc.toString("hh:mm:ss.zzz")));
    ui->tableWidget_mtkagc->setItem(1, 1, new QTableWidgetItem(QString::number(mtk_agc.agcLevel_rfCh1)));
    ui->tableWidget_mtkagc->setItem(2, 1, new QTableWidgetItem(QString::number(mtk_agc.agcLevel_rfCh2)));
    ui->tableWidget_mtkagc->setItem(3, 1, new QTableWidgetItem(QString::number(mtk_agc.agcLevel_rfCh3)));
    ui->tableWidget_mtkagc->setItem(4, 1, new QTableWidgetItem(QString::number(mtk_agc.agcLevel_rfCh4)));
    ui->tableWidget_mtkagc->setItem(5, 1, new QTableWidgetItem(QString::number(mtk_agc.rfState)));
    ui->tableWidget_mtkagc->setItem(6, 1, new QTableWidgetItem(QString::number(mtk_agc.avgSignal)));
    ui->tableWidget_mtkagc->setItem(7, 1, new QTableWidgetItem(mtk_agc.valid?"TRUE":"FALSE"));

}

void gpsWindow::sl_gllData(GllData gll)
{
    ui->tableWidget_gll->setItem(0, 1, new QTableWidgetItem(gll.utc.toString("hh:mm:ss.zzz")));
    ui->tableWidget_gll->setItem(1, 1, new QTableWidgetItem(QString::number(gll.lat)));
    ui->tableWidget_gll->setItem(2, 1, new QTableWidgetItem(QString::number(gll.lon)));
    ui->tableWidget_gll->setItem(3, 1, new QTableWidgetItem(gll.talker));
    ui->tableWidget_gll->setItem(4, 1, new QTableWidgetItem(gll.status));
    ui->tableWidget_gll->setItem(5, 1, new QTableWidgetItem(gll.mode));
    ui->tableWidget_gll->setItem(6, 1, new QTableWidgetItem(gll.valid?"TRUE":"FALSE"));

}

void gpsWindow::sl_vtgData(VtgData vtg)
{

    ui->tableWidget_vtg->setItem(0, 1, new QTableWidgetItem(vtg.talker));
    ui->tableWidget_vtg->setItem(1, 1, new QTableWidgetItem(QString::number(vtg.courseTrue)));
    ui->tableWidget_vtg->setItem(2, 1, new QTableWidgetItem(QString::number(vtg.speedKmh)));
    ui->tableWidget_vtg->setItem(3, 1, new QTableWidgetItem(vtg.mode));
    ui->tableWidget_vtg->setItem(4, 1, new QTableWidgetItem(vtg.valid?"TRUE":"FALSE"));

}

void gpsWindow::sl_skyPlot(GnssSystem gSys, QVector<SkySat> listSkySat)
{
    obj_skyPlot->m_updateConstellation(gSys,listSkySat);
}

void gpsWindow::sl_mtkmpe1(PmtkMpe1 mtk_mpe1)
{
    ui->tableWidget_mtkmpe1->setItem(0, 1, new QTableWidgetItem(QString::number(mtk_mpe1.heading,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(1, 1, new QTableWidgetItem(QString::number(mtk_mpe1.roll,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(2, 1, new QTableWidgetItem(QString::number(mtk_mpe1.pitch,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(3, 1, new QTableWidgetItem(QString::number(mtk_mpe1.yaw,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(4, 1, new QTableWidgetItem(QString::number(mtk_mpe1.vx,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(5, 1, new QTableWidgetItem(QString::number(mtk_mpe1.vy,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(6, 1, new QTableWidgetItem(QString::number(mtk_mpe1.vz,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(7, 1, new QTableWidgetItem(QString::number(mtk_mpe1.ax,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(8, 1, new QTableWidgetItem(QString::number(mtk_mpe1.ay,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(9, 1, new QTableWidgetItem(QString::number(mtk_mpe1.az,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(10, 1, new QTableWidgetItem(QString::number(mtk_mpe1.gx,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(11, 1, new QTableWidgetItem(QString::number(mtk_mpe1.gy,'f', 2)));
    ui->tableWidget_mtkmpe1->setItem(12, 1, new QTableWidgetItem(QString::number(mtk_mpe1.s1)));
    ui->tableWidget_mtkmpe1->setItem(13, 1, new QTableWidgetItem(QString::number(mtk_mpe1.s2)));
    ui->tableWidget_mtkmpe1->setItem(14, 1, new QTableWidgetItem(QString::number(mtk_mpe1.s3)));
    ui->tableWidget_mtkmpe1->setItem(15, 1, new QTableWidgetItem(QString::number(mtk_mpe1.s4)));
    ui->tableWidget_mtkmpe1->setItem(16, 1, new QTableWidgetItem(mtk_mpe1.valid?"TRUE":"FALSE"));

}

