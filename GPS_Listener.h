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
#include <StructData.h>

class GPS_Listener : public QObject
{
    Q_OBJECT
public:
    GPS_Listener(QObject *parent = nullptr);
    ~GPS_Listener();
private slots:
    void sl_positionUpdated(const QGeoPositionInfo &info);
    void sl_onError(QGeoPositionInfoSource::Error error);
    void sl_startNmeaListener();
    void sl_stopNmeaListener();
public slots:
     Q_INVOKABLE void NMEAData_Received(const QString &nmea_data);
signals:
    void si_error_msg(QString);
    void si_Positions(gps_pos  );
    void si_gnAccuracy(double ,double ,double ,double );
    void si_ggaData(GgaData);
    void si_rmcData(RmcData);
    void si_mtkPos1(PmtkPos1);
    void si_mtkPos2(PmtkPos2);
    void si_mtkAgc(PmtkAgc);
    void si_gllData(GllData);
    void si_vtgData(VtgData);
    void si_skyPlot(GnssSystem, QVector<SkySat>);
private:
    QGeoPositionInfoSource *source;
    QString m_gpsLogFile();
    QString strLogFile;
    QJniObject m_nmeaListener;   //  keep Java object alive
    double m_parseLatLon(const QString &value, const QString &dir);
    QMap<QString, QVector<SatelliteInfo>> gsvAccumulator;
    QMap<QString, int> gsvExpected;
    QString m_GetConstellationString(QString talker_id);
    GnssSystem m_GetConstellationClassId(QString conString);
    void m_handleGSV(const QString &nmea);
    QVector<SatelliteInfo> m_parseGSV(const QString &nmea);
    GgaData m_parseGGA(const QString &nmea);
    RmcData m_parseRMC(const QString &nmea);
    PmtkPos1 m_parsePmtkPos1(const QString &nmea);
    PmtkPos2 m_parsePmtkPos2(const QString &nmea);
    PmtkAgc m_parsePmtkAgc(const QString &nmea);
    PmtkMpe1 m_parsePmtkMpe1(const QString &nmea);
    GllData m_parseGLL(const QString &nmea);
    VtgData m_parseVTG(const QString &nmea);
    GsaData m_parseGSA(const QString &nmea);
};

#endif // GPS_LISTENER_H
