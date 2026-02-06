#ifndef GPS_LISTENER_H
#define GPS_LISTENER_H
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <QGeoPositionInfo>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <jni.h>
#include <QJniObject>
#include <QTimer>

struct gps_pos
{
    double lat;
    double lon;
    double speed;
    double acc;
};

struct SatelliteInfo
{
    QString constellation;  // GPS / GLONASS / GALILEO / BEIDOU
    int prn = 0;
    int elevation = 0;      // degrees
    int azimuth = 0;        // degrees
    int snr = 0;            // dB-Hz
};

struct GgaData
{
    QString utcTime;
    double latitude = 0.0;
    double longitude = 0.0;
    int fixQuality = 0;
    int satellites = 0;
    double hdop = 0.0;
    double altitude = 0.0;
    bool valid = false;
};

struct RmcData
{
    QString talker;   // GP or GN
    QString utcTime;
    QChar status;     // A or V
    double latitude = 0.0;
    double longitude = 0.0;
    double speedKmh = 0.0;
    double course = 0.0;
    QString date;
    QChar mode;       // A, D, N
    bool valid = false;
};



class GPS_Listener : public QObject
{
    Q_OBJECT
public:
    GPS_Listener(QObject *parent = nullptr);
    ~GPS_Listener();
private slots:
    void positionUpdated(const QGeoPositionInfo &info);
    void onError(QGeoPositionInfoSource::Error error);
    void sl_startNmeaListener();
    void sl_stopNmeaListener();
public slots:
     Q_INVOKABLE void NMEAData_Received(const QString &nmea_data);
signals:
    void si_error_msg(QString);
    void si_Positions(gps_pos  );
private:
    QGeoPositionInfoSource *source;
    QString gpsLogFile();
    QJniObject m_nmeaListener;   // 🔑 keep Java object alive
    double parseLatLon(const QString &value, const QString &dir);
    QMap<QString, QVector<SatelliteInfo>> gsvAccumulator;
    QMap<QString, int> gsvExpected;
    void handleGSV(const QString &nmea);
    QVector<SatelliteInfo> parseGSV(const QString &nmea);
    GgaData parseGGA(const QString &nmea);
    RmcData parseRMC(const QString &nmea);

};

#endif // GPS_LISTENER_H
