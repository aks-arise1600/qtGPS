#ifndef GPS_LISTENER_H
#define GPS_LISTENER_H
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>

struct gps_pos
{
    double lat;
    double lon;
    double speed;
    double acc;
};

class GPS_Listener : public QObject
{
    Q_OBJECT
public:
    GPS_Listener(QObject *parent = nullptr);
private slots:
    void positionUpdated(const QGeoPositionInfo &info);
    void onError(QGeoPositionInfoSource::Error error);
signals:
    void si_error_msg(QString);
    void si_Positions(gps_pos  );
private:
    QGeoPositionInfoSource *source;
    QString gpsLogFile();
};

#endif // GPS_LISTENER_H
