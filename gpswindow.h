#ifndef GPSWINDOW_H
#define GPSWINDOW_H

#include <QMainWindow>
#include <GPS_Listener.h>
#include <gsv_form.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <device_info.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class gpsWindow;
}
QT_END_NAMESPACE

class gpsWindow : public QMainWindow
{
    Q_OBJECT

public:
    gpsWindow(QWidget *parent = nullptr);
    ~gpsWindow();
    void m_postData(const QString &serverUrl, QJsonObject payload);
private slots:
    void sl_Positions(gps_pos gData );
    void sl_DeviceInfo();
    void on_pushButton_Close_clicked();
    void sl_error_msg(QString msg);
    void sl_gnAccuracy(double hAcc,double vAcc,double sAcc,double bAcc);
    void sl_ggaData(GgaData gga);
    void sl_rmcData(RmcData rmc);
    void sl_mtkPos1(PmtkPos1 mtk_pos1);
    void sl_mtkPos2(PmtkPos2 mtk_pos2);
    void sl_mtkAgc(PmtkAgc mtk_agc);
    void sl_gllData(GllData gll);
    void sl_vtgData(VtgData vtg);
    void sl_skyPlot(GnssSystem gSys, QVector<SkySat> listSkySat);
    void sl_mtkmpe1(PmtkMpe1 mtk_mpe1);
private:
    Ui::gpsWindow *ui;
    GPS_Listener * obj_gps;
    GSV_Form *obj_skyPlot;
    int iCount = 0;
    Device_Info *obj_devInfo = nullptr;
    DeviceInfoData diData;

};
#endif // GPSWINDOW_H
