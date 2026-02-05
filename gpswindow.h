#ifndef GPSWINDOW_H
#define GPSWINDOW_H

#include <QMainWindow>
#include <GPS_Listener.h>

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
private slots:
    void sl_Positions(gps_pos gData );
    void on_pushButton_Close_clicked();
    void sl_error_msg(QString msg);
private:
    Ui::gpsWindow *ui;
    GPS_Listener * obj_gps;
};
#endif // GPSWINDOW_H
