#include "gpswindow.h"
#include "ui_gpswindow.h"

gpsWindow::gpsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::gpsWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/compass.png"));

    obj_gps = new GPS_Listener(this);
    connect(obj_gps,SIGNAL(si_Positions(gps_pos)),this, SLOT(sl_Positions(gps_pos)));
    connect(obj_gps,SIGNAL(si_error_msg(QString)),this, SLOT(sl_error_msg(QString)));

}

gpsWindow::~gpsWindow()
{
    delete ui;
}

void gpsWindow::sl_Positions(gps_pos gData)
{
    ui->label_Lat->setText(QString::number(gData.lat));
    ui->label_Long->setText(QString::number(gData.lon));
    ui->label_speed->setText(QString::number(gData.speed));
    ui->label_acc->setText(QString::number(gData.acc));

}

void gpsWindow::on_pushButton_Close_clicked()
{
    this->close();
}

void gpsWindow::sl_error_msg(QString msg)
{
    ui->plainTextEdit->appendPlainText(msg);
}

