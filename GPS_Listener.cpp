#include "GPS_Listener.h"

static GPS_Listener *g_gpsListener = nullptr;

extern "C"
    JNIEXPORT void JNICALL
    Java_org_example_gps_NmeaListener_nativeOnNmea(
        JNIEnv *env,
        jclass,
        jstring nmea)
{
    if (!g_gpsListener) {
        qWarning() << "GPS_Listener callback ignored: bridge not ready";
        return;
    }

    const char *c = env->GetStringUTFChars(nmea, nullptr);
    QString sentence = QString::fromUtf8(c);
    env->ReleaseStringUTFChars(nmea, c);



    QMetaObject::invokeMethod(
        g_gpsListener,
        "NMEAData_Received",
        Qt::QueuedConnection,
        Q_ARG(QString, sentence)
        );
}

GPS_Listener::GPS_Listener(QObject *parent)
{
    Q_UNUSED(parent)
    g_gpsListener = this;
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
    QTimer::singleShot(5000,this,SLOT(sl_startNmeaListener()));
}

GPS_Listener::~GPS_Listener()
{
    if (g_gpsListener == this)
        g_gpsListener = nullptr;
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

void GPS_Listener::sl_startNmeaListener()
{
#ifdef Q_OS_ANDROID
    if (m_nmeaListener.isValid()) {
        qDebug() << "NMEA listener already running";
        return;
    }

    QJniObject activity =
        QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;"
            );

    if (!activity.isValid()) {
        qWarning() << "No Android activity";
        return;
    }

    m_nmeaListener = QJniObject(
        "org/example/gps/NmeaListener",
        "(Landroid/content/Context;)V",
        activity.object<jobject>()
        );

    if (!m_nmeaListener.isValid()) {
        qWarning() << "Failed to create NmeaListener";
        return;
    }

    m_nmeaListener.callMethod<void>("start");
#endif
}

void GPS_Listener::sl_stopNmeaListener()
{
#ifdef Q_OS_ANDROID
    if (m_nmeaListener.isValid()) {
        m_nmeaListener.callMethod<void>("stop");
        m_nmeaListener = QJniObject(); // release ref
    }
#endif
}

void GPS_Listener::NMEAData_Received(const QString &nmea_data)
{
    /**
     *  $GPRMC,hhmmss.sss,status,lat,NS,lon,EW,sog,cog,ddmmyy,magvar,magvarEW,mode*checksum
     *  $GPGGA,UTC_time(),lat, N/S,lon, E/W,Fix_quality,Num_of_satellites,HDOP,Altitude, M,Geoid_separation, M,DGPS_age,DGPS_station_ID
     *  $GPGLL,lat,N/S,lon,E/W,time,status,mode*CS
     *  $GPVTG,cog,T,cog,M,sog,N,sog,K,mode*CS
     *  $GLGSV,total_msgs,msg_num,sats_in_view,...
     *
     *  **/
    // Example filtering
    if (nmea_data.startsWith("$PMTKLCPOS1"))
    {
        QStringList f = nmea_data.split(',');
        if (f.size() > 5 && f[5] == "1") {
            double lat = f[2].toDouble();
            double lon = f[3].toDouble();

            qDebug() << "MTK POS1 FIX Lat:" << lat << "Lon:" << lon;
        }
    }
    else if (nmea_data.startsWith("$PMTKLCPOS2"))
    {
        QString line = nmea_data;
        int star = line.indexOf('*');
        if (star > 0)
            line = line.left(star);

        QStringList f = line.split(',');

        if (f.size() >= 9) {
            double lat = f[2].toDouble();
            double lon = f[3].toDouble();
            int sats   = f[8].toInt();

            qDebug() << "MTK POS2"
                     << "Lat:" << lat
                     << "Lon:" << lon
                     << "Satellites:" << sats;
        }
    }
    else if (nmea_data.startsWith("$GNACCURACY"))
    {
        qDebug() << "ACCURACY :";
        QString line = nmea_data;
        int star = line.indexOf('*');
        if (star > 0)
            line = line.left(star);

        QStringList f = line.split(',');

        if (f.size() >= 5) {
            double hAcc = f[1].toDouble();
            double vAcc = f[2].toDouble();
            double sAcc = f[3].toDouble();
            double bAcc = f[4].toDouble();

            qDebug() << "  H(m):" << hAcc
                     << "V(m):" << vAcc
                     << "Speed(m/s):" << sAcc
                     << "Bearing(deg):" << bAcc;
        }
    }
    else if (nmea_data.startsWith("$GNGGA") || nmea_data.startsWith("$GPGGA"))
    {
        qDebug() << "GGA:";
        auto gga = parseGGA(nmea_data);

        qDebug() << "  UTC:" << gga.utcTime
                 << "Lat:" << gga.latitude
                 << "Lon:" << gga.longitude
                 << "Fix:" << gga.fixQuality
                 << "Sats:" << gga.satellites
                 << "HDOP:" << gga.hdop
                 << "Alt:" << gga.altitude
                 << "Valid:" << gga.valid;

    }
    else if (nmea_data.startsWith("$GNRMC") || nmea_data.startsWith("$GPRMC"))
    {
        qDebug() << "RMC:";

        auto rmc = parseRMC(nmea_data);

        qDebug() << "  UTC:" << rmc.utcTime
                 << "Lat:" << rmc.latitude
                 << "Lon:" << rmc.longitude
                 << "Speed:" << rmc.speedKmh
                 << "Course:" << rmc.course
                 << "Date:" << rmc.date
                 << "Mode:" << rmc.mode
                 << "Stat:" << rmc.status
                 << "Valid:" << rmc.valid;

    }
    else if (nmea_data.startsWith("$P"))
    {
        qDebug() << "Proprietary NMEA:" << nmea_data.trimmed();
    }
    else if (nmea_data.startsWith("$GPGSV") || nmea_data.startsWith("$GLGSV") ||
        nmea_data.startsWith("$GAGSV") || nmea_data.startsWith("$GBGSV") ||
        nmea_data.startsWith("$GNGSV"))
    {
        qDebug() << "GSV:";
        handleGSV(nmea_data);
    }
    else
    {
        qDebug() <<"UNKN:"<< nmea_data;
    }

    if(nmea_data.size())
        emit si_error_msg(nmea_data);
}
double GPS_Listener::parseLatLon(const QString &value, const QString &dir)
{
    if (value.isEmpty() || dir.isEmpty())
        return 0.0;

    // Find decimal point
    int dot = value.indexOf('.');
    if (dot < 0)
        return 0.0;

    // Degrees are everything before the last 2 digits before '.'
    int degLen = dot - 2;
    if (degLen <= 0)
        return 0.0;

    double degrees = value.left(degLen).toDouble();
    double minutes = value.mid(degLen).toDouble();

    double decimal = degrees + (minutes / 60.0);

    // South & West are negative
    if (dir == "S" || dir == "W")
        decimal = -decimal;

    return decimal;
}



void GPS_Listener::handleGSV(const QString &nmea)
{
    QString line = nmea;
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f = line.split(',');
    if (f.size() < 4)
    {
        qDebug()<<"  ignore()";
        return;
    }



    QString talker = f[0].mid(1, 2);
    int totalMsgs = f[1].toInt();
    int msgNum    = f[2].toInt();

    auto sats = parseGSV(nmea);

    gsvAccumulator[talker] += sats;
    gsvExpected[talker] = totalMsgs;

    if (msgNum == totalMsgs)
    {
        // Full constellation snapshot ready
        auto allSats = gsvAccumulator[talker];

        qDebug() << "  Constellation:" << talker
                 << "Visible sats:" << allSats.size();

        for (const auto &s : allSats)
        {
            qDebug() <<"    "<< s.constellation
                     << "PRN:" << s.prn
                     << "Elev:" << s.elevation
                     << "Az:" << s.azimuth
                     << "SNR:" << s.snr;
        }

        gsvAccumulator[talker].clear();
    }
    else
        qDebug() <<"  msgNum="<<msgNum<<", totalMsgs="<<totalMsgs;
}

QVector<SatelliteInfo> GPS_Listener::parseGSV(const QString &nmea)
{
    QVector<SatelliteInfo> sats;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f = line.split(',');
    if (f.size() < 4)
        return sats;

    // Talker IDs
    // GP, GL, GA, GB, GN, AI, QZ, GI, CD, EC, BD, GN, GQ, HC, HE, II, IN, LC, Pxxx, PQ, SD, ST, TI, YX, WI

    QString talker = f[0].mid(1, 2);

    QString constellation;
    if (talker == "GP") constellation = "GPS";
    else if (talker == "GL") constellation = "GLONASS";
    else if (talker == "GA") constellation = "GALILEO";
    else if (talker == "GB" || talker == "BD") constellation = "BEIDOU";
    else constellation = "MIXED";

    // Satellite blocks start at index 4
    for (int i = 4; i + 3 < f.size(); i += 4) {
        if (f[i].isEmpty())
            continue;

        SatelliteInfo s;
        s.constellation = constellation;
        s.prn       = f[i].toInt();
        s.elevation = f[i + 1].toInt();
        s.azimuth   = f[i + 2].toInt();
        s.snr       = f[i + 3].toInt();

        sats.append(s);
    }

    return sats;
}

GgaData GPS_Listener::parseGGA(const QString &nmea)
{
    GgaData gga;

    QString line = nmea.trimmed();

    // Remove checksum
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f = line.split(',');

    if (f.size() < 10)
        return gga;

    gga.utcTime    = f[1];
    gga.fixQuality = f[6].toInt();
    gga.satellites = f[7].toInt();
    gga.hdop       = f[8].isEmpty() ? 0.0 : f[8].toDouble();
    gga.altitude   = f[9].toDouble();

    // Parse lat/lon only if present
    if (!f[2].isEmpty() && !f[4].isEmpty()) {
        gga.latitude  = parseLatLon(f[2], f[3]);
        gga.longitude = parseLatLon(f[4], f[5]);
    }

    // Valid fix only if fixQuality > 0 and sats >= 4
    gga.valid = (gga.fixQuality > 0 && gga.satellites >= 4);

    return gga;
}

RmcData GPS_Listener::parseRMC(const QString &nmea)
{
    RmcData rmc;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f = line.split(',');
    if (f.size() < 10)
        return rmc;

    rmc.talker  = f[0].mid(1, 2); // GP / GN
    rmc.utcTime = f[1];
    rmc.status  = f[2].isEmpty() ? 'V' : f[2][0];
    rmc.date    = f[9];
    rmc.mode    = (f.size() > 12 && !f[12].isEmpty()) ? f[12][0] : 'N';

    if (rmc.status == 'A')
    {
        rmc.latitude  = parseLatLon(f[3], f[4]);
        rmc.longitude = parseLatLon(f[5], f[6]);
        rmc.speedKmh  = f[7].toDouble() * 1.852; // knots → km/h
        rmc.course    = f[8].toDouble();
        rmc.valid = true;
    }

    return rmc;
}
