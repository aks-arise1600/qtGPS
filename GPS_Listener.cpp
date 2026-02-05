#include "GPS_Listener.h"


GPS_Listener::GPS_Listener(QObject *parent)
{
    Q_UNUSED(parent)

    source = QGeoPositionInfoSource::createDefaultSource(this);
    if (!source) {
        qWarning() << "No GPS source";
        emit si_error_msg("No GPS source");
    }

    // High update rate
    source->setUpdateInterval(1000); // 1 second

    // Prefer GPS over network
    //source->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);

    connect(source, &QGeoPositionInfoSource::positionUpdated, this, &GPS_Listener::positionUpdated);
    connect(source, &QGeoPositionInfoSource::errorOccurred, this, &GPS_Listener::onError);

    source->startUpdates();
}

void GPS_Listener::positionUpdated(const QGeoPositionInfo &info)
{
    if (!info.isValid()) return;
    gps_pos obj_gpsData;

    obj_gpsData.lat = info.coordinate().latitude();
    obj_gpsData.lon = info.coordinate().longitude();
    obj_gpsData.speed = info.attribute(QGeoPositionInfo::GroundSpeed); // m/s
    obj_gpsData.acc = info.attribute(QGeoPositionInfo::HorizontalAccuracy) *3.6;

    qDebug() << "Lat:" << obj_gpsData.lat
             << "Lon:" << obj_gpsData.lon
             << "Speed(km/h):" << obj_gpsData.speed
             << "Accuracy(m):" << obj_gpsData.acc;

    emit si_Positions(obj_gpsData );



    QFile file(gpsLogFile());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open GPS log file";
        emit si_error_msg("Failed to open GPS log file");
        return;
    }

    QTextStream out(&file);

    out << QDateTime::currentDateTime().toString(Qt::ISODate) << ", "
        << obj_gpsData.lat << ", "
        << obj_gpsData.lon << ", "
        << obj_gpsData.speed <<", "
        << obj_gpsData.acc
        << "\n";

    file.close();
}
void GPS_Listener::onError(QGeoPositionInfoSource::Error error)
{
    qWarning() << "GPS error:" << error;
}
QString GPS_Listener::gpsLogFile()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QDir().mkpath(dir);  // ensure directory exists
    emit si_error_msg(dir + "/gps_log.txt");
    return dir + "/gps_log.txt";
}
