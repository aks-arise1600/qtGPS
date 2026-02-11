#include "gpswindow.h"
#include "ui_gpswindow.h"

gpsWindow::gpsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gpsWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/compass.png"));

    ui->tableWidget_GNACCURACY->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_QgpsPos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_gga->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_rmc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkpos1->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkpos2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_mtkagc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_gll->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_vtg->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableWidget_GNACCURACY->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_rmc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkpos1->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkpos2->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_mtkagc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


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

    obj_skyPlot = new GSV_Form(this);
    ui->skyPlotLayout->addWidget(obj_skyPlot);

}

gpsWindow::~gpsWindow()
{
    delete ui;
}

void gpsWindow::sl_Positions(gps_pos gData)
{
    ui->tableWidget_QgpsPos->setItem(0, 1, new QTableWidgetItem(QString::number(gData.lat)));
    ui->tableWidget_QgpsPos->setItem(1, 1, new QTableWidgetItem(QString::number(gData.lon)));
    ui->tableWidget_QgpsPos->setItem(2, 1, new QTableWidgetItem(QString::number(gData.speed)));
    ui->tableWidget_QgpsPos->setItem(3, 1, new QTableWidgetItem(QString::number(gData.acc)));

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

