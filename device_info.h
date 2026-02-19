#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H
#include <QObject>
#include <QDebug>
#include <QJniObject>

struct DeviceInfoData
{
    QString board;
    QString product;
    QString manufacturer;
    QString model;
    QString android_id;
    QString dev_id;
};

class Device_Info :public QObject
{
    Q_OBJECT
public:
    Device_Info();
    QJniObject m_getActivity();
    QString m_androidBoard();
    QString m_androidProduct();
    QString m_androidManufacturer();
    QString m_androidModel();
    QString m_androidId();
    QString m_androidDeviceId();
};

#endif // DEVICE_INFO_H
