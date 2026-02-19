#ifndef STRUCTDATA_H
#define STRUCTDATA_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QElapsedTimer>

struct gps_pos
{
    double lat;
    double lon;
    double alt = 0.0;
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
    QTime utcTime;
    QString talker;   // GP or GN
    double lat = 0.0;
    double lon = 0.0;
    int fixQuality = 0;
    int satellites = 0;
    double hdop = 0.0;
    double altitude = 0.0;
    bool valid = false;
};

struct RmcData
{
    QString talker;   // GP or GN
    QDateTime utc;
    QChar status;     // A or V
    double latitude = 0.0;
    double longitude = 0.0;
    double speedKmh = 0.0;
    double course = 0.0;
    QChar mode;       // A, D, N
    bool valid = false;
};

enum class PmtkClass {
    Ignore,      // silent
    Info,        // low importance
    Diagnostic,  // health metrics
    Error        // actionable problem
};


struct PmtkPos1
{
    QDateTime utc;
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    double hVelocity = 0.0; // m/s
    double vVelocity = 0.0; // m/s
    bool valid = false;
};

struct PmtkPos2
{
    QDateTime utc;
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    int fixType = 0;          // vendor-specific
    int fwOrClockId = 0;
    double clockCounter = 0;  // vendor counter
    int satellites = 0;       // visible/used
    bool valid = false;       // derived
};

struct PmtkAgc
{
    QTime utc;
    int agcLevel_rfCh1 = 0;
    int agcLevel_rfCh2 = 0;
    int agcLevel_rfCh3 = 0;
    int agcLevel_rfCh4 = 0;
    int rfState = 0;
    int avgSignal = 0;
    bool valid = false;
};

struct PmtkMpe1
{
    double heading = 0.0;
    double pitch = 0.0;
    double roll = 0.0;
    double yaw = 0.0;

    double vx = 0.0, vy = 0.0, vz = 0.0;
    double ax = 0.0, ay = 0.0, az = 0.0;
    double gx = 0.0, gy = 0.0;

    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;

    bool valid = false;
};

struct GllData
{
    QString talker;   // GP / GN
    QTime utc;
    double lat = 0.0;
    double lon = 0.0;
    QChar status = 'V';
    QChar mode = 'N';
    bool valid = false;
};

struct VtgData
{
    QString talker;   // GP / GN
    double courseTrue = 0.0;
    double speedKmh = 0.0;
    QChar mode = 'N';
    bool valid = false;
};

struct GsaData
{
    QString talker;          // GP / GN
    QChar mode1;             // A / M
    int fixType = 1;         // 1,2,3
    QVector<int> satIds;     // satellites used
    double pdop = 99.99;
    double hdop = 99.99;
    double vdop = 99.99;
    int systemId = 0;
    bool valid = false;
};

enum class GnssSystem {
    GPS,
    GLONASS,
    BEIDOU,
    GALILEO,
    QZSS,
    IRNSS_NAVIC,
    UNKNOWN
};

struct SkySat
{
    GnssSystem system;
    int prn = 0;
    double elevation = 0.0; // degrees (0–90)
    double azimuth = 0.0;   // degrees (0–360)
    double snr = 0.0;       // optional
    QElapsedTimer lastSeen;
};

#endif // STRUCTDATA_H
