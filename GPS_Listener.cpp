#include "GPS_Listener.h"

static GPS_Listener *g_gpsListener = nullptr;

QString doc_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/GPS_Log";

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

    connect(source, &QGeoPositionInfoSource::positionUpdated, this, &GPS_Listener::sl_positionUpdated);
    connect(source, &QGeoPositionInfoSource::errorOccurred, this, &GPS_Listener::sl_onError);
    source->startUpdates();
    QTimer::singleShot(5000,this,SLOT(sl_startNmeaListener()));

    strLogFile = m_gpsLogFile();
}

GPS_Listener::~GPS_Listener()
{
    if (g_gpsListener == this)
        g_gpsListener = nullptr;
}

void GPS_Listener::sl_positionUpdated(const QGeoPositionInfo &info)
{
    if (!info.isValid())
    {
        qDebug()<<"Inavlid Info";
        return;
    }
    gps_pos obj_gpsData;

    obj_gpsData.lat = info.coordinate().latitude();
    obj_gpsData.lon = info.coordinate().longitude();
    obj_gpsData.alt = info.coordinate().altitude();
    obj_gpsData.speed = info.attribute(QGeoPositionInfo::GroundSpeed); // m/s
    obj_gpsData.acc = info.attribute(QGeoPositionInfo::HorizontalAccuracy) *3.6;

    qDebug() << "Lat:" << obj_gpsData.lat
             << "Lon:" << obj_gpsData.lon
             << "Alt:" << obj_gpsData.alt
             << "Speed(km/h):" << obj_gpsData.speed
             << "Accuracy(m):" << obj_gpsData.acc;

    emit si_Positions(obj_gpsData );



    QFile file(strLogFile);

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
void GPS_Listener::sl_onError(QGeoPositionInfoSource::Error error)
{
    qWarning() << "GPS error:" << error;
}
QString GPS_Listener::m_gpsLogFile()
{

    QDir().mkpath(doc_dir);  // ensure directory exists
    QString absoluteFname = doc_dir + "/"+QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")+"_gps_log.txt";
    emit si_error_msg(absoluteFname);
    qDebug()<<"absoluteFname"<<absoluteFname;
    return absoluteFname;
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

bool GPS_Listener::m_isValidChecksum(const QByteArray &sentence)
{
    // sentence must start with '$' and contain '*'
    int star = sentence.indexOf('*');
    if (sentence.isEmpty() || sentence[0] != '$' || star < 0)
        return false;

    // Need exactly 2 hex chars after '*'
    if (sentence.size() < star + 3)
        return false;

    quint8 checksum = 0;

    // XOR from after '$' up to before '*'
    for (int i = 1; i < star; ++i)
        checksum ^= static_cast<quint8>(sentence[i]);

    bool ok = false;
    quint8 sent =
        sentence.mid(star + 1, 2).toUInt(&ok, 16);

    return ok && (checksum == sent);
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

    static QByteArray buffer ;
    buffer.append(nmea_data.toLatin1());

    while (true)
    {
        int start = buffer.indexOf('$');
        if (start < 0) {
            buffer.clear();
            return;
        }

        if (start > 0)
            buffer.remove(0, start);

        int star = buffer.indexOf('*');
        if (star < 0)
            return;

        if (buffer.size() < star + 3)
            return;

        QByteArray sentence = buffer.left(star + 3);
        buffer.remove(0, star + 3);

        // CHECKSUM VALIDATION
        if (!m_isValidChecksum(sentence))
        {
            qWarning() << "Bad checksum:" << sentence;
            continue;   // DROP corrupt packet
        }

        QString str_nmea = QString::fromLatin1(sentence);
        str_nmea.remove('\r');
        str_nmea.remove('\n');

        m_parseNMEA_Sentence(str_nmea);
    }

    qDebug()<<"buffer"<<buffer;
}

void GPS_Listener::m_parseNMEA_Sentence(QString nmea_data)
{
    // Example filtering
    if (nmea_data.startsWith("$GNACCURACY"))
    {
        qDebug() << "ACCURACY :"<< nmea_data;
        QString line = nmea_data;
        int star = line.indexOf('*');
        if (star > 0)
            line = line.left(star);

        QStringList f_list = line.split(',');

        if (f_list.size() >= 5)
        {
            double hAcc = f_list[1].toDouble();
            double vAcc = f_list[2].toDouble();
            double sAcc = f_list[3].toDouble();
            double bAcc = f_list[4].toDouble();

            qDebug() << "  H(m):" << hAcc
                     << "V(m):" << vAcc
                     << "Speed(m/s):" << sAcc
                     << "Bearing(deg):" << bAcc;

            emit si_gnAccuracy(hAcc,vAcc,sAcc,bAcc);
        }
    }
    else if (nmea_data.startsWith("$GNGGA") || nmea_data.startsWith("$GPGGA"))
    {
        qDebug() << "GGA:" << nmea_data;
        auto gga = m_parseGGA(nmea_data);

        qDebug() << "  Time:" << gga.utcTime
                 << "Talker:" << gga.talker
                 << "Lat:" << gga.lat
                 << "Lon:" << gga.lon
                 << "Fix:" << gga.fixQuality
                 << "Sats:" << gga.satellites
                 << "HDOP:" << gga.hdop
                 << "Alt:" << gga.altitude
                 << "Valid:" << gga.valid;

        emit si_ggaData(gga);

    }
    else if (nmea_data.startsWith("$GNRMC") || nmea_data.startsWith("$GPRMC"))
    {
        qDebug() << "RMC:" << nmea_data;

        auto rmc = m_parseRMC(nmea_data);

        qDebug() << "  DateTime:" << rmc.utc
                 << "Talker:" << rmc.talker
                 << "Lat:" << rmc.latitude
                 << "Lon:" << rmc.longitude
                 << "Speed:" << rmc.speedKmh
                 << "Course:" << rmc.course
                 << "Mode:" << rmc.mode
                 << "Stat:" << rmc.status
                 << "Valid:" << rmc.valid;

        emit si_rmcData(rmc);

    }
    else if (nmea_data.startsWith("$PMTK"))
    {
        if (nmea_data.startsWith("$PMTKAGC"))
        {
            qDebug() << "MTK AGC:" << nmea_data;
            PmtkAgc agc = m_parsePmtkAgc(nmea_data);
            qDebug() << "    Time:" << agc.utc
                     << "agcLevel_rfCh1:" << agc.agcLevel_rfCh1
                     << "agcLevel_rfCh2:" << agc.agcLevel_rfCh2
                     << "agcLevel_rfCh3:" << agc.agcLevel_rfCh3
                     << "agcLevel_rfCh4:" << agc.agcLevel_rfCh4
                     << "rfState:" << agc.rfState
                     << "avgSignal:" << agc.avgSignal
                     << "Valid:" << agc.valid;

            emit si_mtkAgc(agc);
        }
        else if (nmea_data.startsWith("$PMTKLCPOS1"))
        {
            qDebug() << "MTK POS1 FIX:" << nmea_data;
            PmtkPos1 mtkP1 = m_parsePmtkPos1(nmea_data);
            qDebug() << "  Lat:" << mtkP1.lat << "Lon:" << mtkP1.lon
                     << "Alt:"<<mtkP1.alt << "h_velocity:"<<mtkP1.hVelocity
                     << "v_velocity:"<<mtkP1.vVelocity << "Valid:"<<mtkP1.valid
                     << "DateTime:"<< mtkP1.utc;

            emit si_mtkPos1(mtkP1);
        }
        else if (nmea_data.startsWith("$PMTKLCPOS2"))
        {
            qDebug() << "MTK POS2:"<< nmea_data;
            PmtkPos2 mtkP2 = m_parsePmtkPos2(nmea_data);
            qDebug() << "  Lat:" << mtkP2.lat << "Lon:"  << mtkP2.lon
                     << "Alt:"<<mtkP2.alt << "fixType:" << mtkP2.fixType
                     << "fwOrClockId:"<<mtkP2.fwOrClockId << "clockCounter:" << mtkP2.clockCounter
                     << "satellites:"<<mtkP2.satellites << "Valid:"<<mtkP2.valid
                     << "DateTime:"<< mtkP2.utc;

            emit si_mtkPos2(mtkP2);
        }
        else if(nmea_data.startsWith("$PMTKMPE1"))
        {
            qDebug() << "MTK MPE1:"<< nmea_data;
            PmtkMpe1 mpe1 = m_parsePmtkMpe1(nmea_data);
            qDebug() << "  heading:" << mpe1.heading << "pitch:"  << mpe1.pitch
                     << "roll:"<<mpe1.roll << "yaw:" << mpe1.yaw
                     << "vx:"<<mpe1.vx << "vy:"<<mpe1.vy << "vz:"<<mpe1.vz
                     << "ax:"<<mpe1.ax << "ay:"<<mpe1.ay << "az:"<<mpe1.az
                     << "gx:"<<mpe1.gx << "gy:"<<mpe1.gy << "s1:"<< mpe1.s1
                     << "s2:"<< mpe1.s2 << "s3:"<< mpe1.s3 << "s4:"<< mpe1.s4;

            emit si_mtkmpe1(mpe1);
        }
        else
            qDebug() << "MTK (Proprietary NMEA):" << nmea_data.trimmed();
    }
    else if (nmea_data.startsWith("$GPGSV") || nmea_data.startsWith("$GLGSV") ||
             nmea_data.startsWith("$GAGSV") || nmea_data.startsWith("$GBGSV") ||
             nmea_data.startsWith("$GNGSV"))
    {
        qDebug() << "GSV:" << nmea_data;
        m_handleGSV(nmea_data);
    }
    else if (nmea_data.startsWith("$GPGLL") || nmea_data.startsWith("$GNGLL"))
    {
        qDebug() << "GLL:"<< nmea_data;
        GllData gll = m_parseGLL(nmea_data);
        qDebug() << "  UTC:" << gll.utc
                 << "Talker:" << gll.talker
                 << "Latitude:" << gll.lat
                 << "Longitude:" << gll.lon
                 << "Status:" << gll.status
                 << "Mode:" << gll.mode
                 << "Valid:" << gll.valid;

        emit si_gllData(gll);
    }
    else if (nmea_data.startsWith("$GPVTG") || nmea_data.startsWith("$GNVTG"))
    {
        qDebug() << "VTG:"<< nmea_data;
        VtgData vtg = m_parseVTG(nmea_data);
        qDebug() << "  Talker:" << vtg.talker
                 << "courseTrue:" << vtg.courseTrue
                 << "speedKmh:" << vtg.speedKmh
                 << "Mode:" << vtg.mode
                 << "Valid:" << vtg.valid;

        emit si_vtgData(vtg);
    }
    else if (nmea_data.startsWith("$GPGSA") || nmea_data.startsWith("$GNGSA"))
    {
        qDebug() << "GSA:"<< nmea_data;
        GsaData gsa = m_parseGSA(nmea_data);
        qDebug() << "  Talker:" << gsa.talker
                 << "Mode:" << gsa.mode1
                 << "satellites:" << gsa.satIds
                 << "Position DOP:" << gsa.pdop
                 << "Horizontal DOP:" << gsa.hdop
                 << "Vertical DOP:" << gsa.vdop
                 << "Valid:" << gsa.valid;
    }
    else
    {
        qDebug() <<"UNPARSED:"<< nmea_data;
    }

    if(nmea_data.size())
    {
        emit si_error_msg(nmea_data);
        QFile file(strLogFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            qWarning() << "Failed to open file:" << file.errorString();
            return;
        }

        QByteArray msgData = nmea_data.toLatin1();
        msgData.append('\n');
        qint64 written = file.write(msgData);

        if (written != msgData.size()) {
            qWarning() << "Incomplete write:" << written << "bytes";
        }

        file.flush();
        file.close();
    }

}
double GPS_Listener::m_parseLatLon(const QString &value, const QString &dir)
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

QString GPS_Listener::m_GetConstellationString(QString talker_id)
{
    QString constellation;
    if (talker_id == "GP") constellation = "GPS";
    else if (talker_id == "GL") constellation = "GLONASS";
    else if (talker_id == "GA") constellation = "GALILEO";
    else if (talker_id == "GB" || talker_id == "BD") constellation = "BEIDOU";
    else if (talker_id == "GQ") constellation = "QZSS";
    else if (talker_id == "GI") constellation = "IRNSS/NAVIC";
    else constellation = "MIXED";

    return constellation;
}

GnssSystem GPS_Listener::m_GetConstellationClassId(QString conString)
{
    if(conString == "GPS")
        return GnssSystem::GPS;
    else if(conString == "BEIDOU")
        return GnssSystem::BEIDOU;
    else if(conString == "GLONASS")
        return GnssSystem::GLONASS;
    else if(conString == "GALILEO")
        return GnssSystem::GALILEO;
    else if(conString == "QZSS")
        return GnssSystem::QZSS;
    else if(conString == "IRNSS/NAVIC")
        return GnssSystem::IRNSS_NAVIC;

    return GnssSystem::UNKNOWN;
}

void GPS_Listener::m_handleGSV(const QString &nmea)
{
    QString line = nmea;
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 4)
    {
        qDebug()<<"  ignore()";
        return;
    }

    QString talker = f_list[0].mid(1, 2);
    int totalMsgs = f_list[1].toInt();
    int msgNum    = f_list[2].toInt();

    auto sats = m_parseGSV(nmea);

    gsvAccumulator[talker] += sats;
    gsvExpected[talker] = totalMsgs;

    if (msgNum == totalMsgs)
    {
        // Full constellation snapshot ready
        auto allSats = gsvAccumulator[talker];

        qDebug() << "  Constellation:" << talker
                 << "Visible sats:" << allSats.size();

        QVector<SkySat> sats;
        GnssSystem tmpSys;
        for (const auto &s : allSats)
        {
            qDebug() <<"    "<< s.constellation
                     << "PRN:" << s.prn
                     << "Elev:" << s.elevation
                     << "Az:" << s.azimuth
                     << "SNR:" << s.snr;

            SkySat sSat;
            tmpSys = sSat.system = m_GetConstellationClassId(s.constellation);
            sSat.prn =  s.prn;
            sSat.elevation =  s.elevation;
            sSat.azimuth =  s.azimuth;
            sSat.snr =  s.snr;

            sats.append(sSat);
        }

        emit si_skyPlot(tmpSys, sats);
        gsvAccumulator[talker].clear();
    }
    else
        qDebug() <<"  msgNum ="<<msgNum<<", totalMsgs ="<<totalMsgs;
}

QVector<SatelliteInfo> GPS_Listener::m_parseGSV(const QString &nmea)
{
    QVector<SatelliteInfo> sats;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 4)
        return sats;

    // Talker IDs
    // GP, GL, GA, GB, GN, AI, QZ, GI, CD, EC, BD, GN, GQ, HC, HE, II, IN, LC, Pxxx, PQ, SD, ST, TI, YX, WI

    QString talker = f_list[0].mid(1, 2);
    QString constellation = m_GetConstellationString(talker);

    // Satellite blocks start at index 4
    for (int i = 4; i + 3 < f_list.size(); i += 4)
    {
        if (f_list[i].isEmpty())
            continue;

        SatelliteInfo s;
        s.constellation = constellation;
        s.prn       = f_list[i].toInt();
        s.elevation = f_list[i + 1].toInt();
        s.azimuth   = f_list[i + 2].toInt();
        s.snr       = f_list[i + 3].toInt();

        sats.append(s);
    }

    return sats;
}

GgaData GPS_Listener::m_parseGGA(const QString &nmea)
{
    GgaData gga;

    QString line = nmea.trimmed();

    // Remove checksum
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');

    if (f_list.size() < 10)
        return gga;

    gga.talker     = m_GetConstellationString( f_list[0].mid(1, 2)); // GP / GN
    //gga.utcTime    = QTime::fromString(f_list[1], "hhmmss.zzz");//f_list[1];
    QTime tmpTime  = QTime::fromString(f_list[1], "hhmmss.zzz");
    gga.utcTime    = tmpTime.addSecs(5 * 3600 + 30 * 60);
    gga.fixQuality = f_list[6].toInt();
    gga.satellites = f_list[7].toInt();
    gga.hdop       = f_list[8].isEmpty() ? 0.0 : f_list[8].toDouble();
    gga.altitude   = f_list[9].toDouble();

    // Parse lat/lon only if present
    if (!f_list[2].isEmpty() && !f_list[4].isEmpty()) {
        gga.lat  = m_parseLatLon(f_list[2], f_list[3]);
        gga.lon = m_parseLatLon(f_list[4], f_list[5]);
    }

    // Valid fix only if fixQuality > 0 and sats >= 4
    gga.valid = (gga.fixQuality > 0 && gga.satellites >= 4);

    return gga;
}

RmcData GPS_Listener::m_parseRMC(const QString &nmea)
{
    RmcData rmc;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 10)
        return rmc;

    rmc.talker  = m_GetConstellationString( f_list[0].mid(1, 2)); // GP / GN
    // Time & Date
    QTime time = QTime::fromString(f_list[1], "hhmmss.zzz");
    QString ddmmyy = f_list[9];
    int day   = ddmmyy.mid(0, 2).toInt();
    int month = ddmmyy.mid(2, 2).toInt();
    int year  = ddmmyy.mid(4, 2).toInt();
    year += (year >= 80) ? 1900 : 2000;

    QDate date(year, month, day);

    if (time.isValid() && date.isValid())
    {
        //rmc.utc = QDateTime(date, time);
        rmc.utc = QDateTime(date, time, QTimeZone::UTC).toTimeZone(QTimeZone("Asia/Kolkata"));
    }


    rmc.status  = f_list[2].isEmpty() ? 'V' : f_list[2][0];
    rmc.mode    = (f_list.size() > 12 && !f_list[12].isEmpty()) ? f_list[12][0] : 'N';

    rmc.latitude  = m_parseLatLon(f_list[3], f_list[4]);
    rmc.longitude = m_parseLatLon(f_list[5], f_list[6]);
    rmc.speedKmh  = f_list[7].toDouble() * 1.852; // knots → km/h
    rmc.course    = f_list[8].toDouble();

    if (rmc.status == 'A')
    {
        rmc.valid = true;
    }

    return rmc;
}

PmtkPos1 GPS_Listener::m_parsePmtkPos1(const QString &nmea)
{
    PmtkPos1 pos1;

    if (!nmea.startsWith("$PMTKLCPOS1"))
        return pos1;

    QString line = nmea.trimmed();

    // Remove checksum
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 8)
        return pos1;

    // UTC time: yyyyMMddhhmmss.zzz

    QDateTime tmpDT = QDateTime::fromString(f_list[1], "yyyyMMddhhmmss.zzz");
    pos1.utc = QDateTime(tmpDT.date(), tmpDT.time(), QTimeZone::UTC).toTimeZone(QTimeZone("Asia/Kolkata"));

    pos1.lat = f_list[2].toDouble();
    pos1.lon = f_list[3].toDouble();
    pos1.alt = f_list[4].toDouble();

    pos1.valid = (f_list[5] == "1");

    // Velocities (m/s)
    pos1.hVelocity = f_list[6].toDouble();
    pos1.vVelocity = f_list[7].toDouble();

    return pos1;
}


PmtkPos2 GPS_Listener::m_parsePmtkPos2(const QString &nmea)
{
    PmtkPos2 pos2;

    if (!nmea.startsWith("$PMTKLCPOS2"))
        return pos2;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    const QStringList f_list = line.split(',');
    if (f_list.size() < 9)
        return pos2;

    QDateTime tmpDT = QDateTime::fromString(f_list[1], "yyyyMMddhhmmss.zzz");
    pos2.utc = QDateTime(tmpDT.date(), tmpDT.time(), QTimeZone::UTC).toTimeZone(QTimeZone("Asia/Kolkata"));;

    pos2.lat = f_list[2].toDouble();
    pos2.lon = f_list[3].toDouble();
    pos2.alt = f_list[4].toDouble();

    pos2.fixType      = f_list[5].toInt();
    pos2.fwOrClockId  = f_list[6].toInt();
    pos2.clockCounter = f_list[7].toDouble();
    pos2.satellites   = f_list[8].toInt();

    // POS2 validity is diagnostic; derive a sensible flag:
    pos2.valid = (pos2.satellites >= 4);

    return pos2;
}

PmtkAgc GPS_Listener::m_parsePmtkAgc(const QString &nmea)
{
    PmtkAgc agc;

    if (!nmea.startsWith("$PMTKAGC"))
        return agc;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 11)
        return agc;

    QTime tmpTime = QTime::fromString(f_list[1], "hhmmss.zzz");
    agc.utc  = tmpTime.addSecs(5 * 3600 + 30 * 60);
    //agc.utc = QTime::fromString(f_list[1], "hhmmss.zzz");

    // AGC values (number may vary by chipset)
    agc.agcLevel_rfCh1 = f_list[2].toInt();
    agc.agcLevel_rfCh2 = f_list[3].toInt();
    agc.agcLevel_rfCh3 = f_list[4].toInt();
    agc.agcLevel_rfCh4 = f_list[5].toInt();

    agc.rfState   = f_list[9].toInt();
    agc.avgSignal = f_list[10].toInt();
    agc.valid = true;

    return agc;
}

PmtkMpe1 GPS_Listener::m_parsePmtkMpe1(const QString &nmea)
{
    PmtkMpe1 mpe1;

    if (!nmea.startsWith("$PMTKMPE1"))
        return mpe1;

    QString line = nmea.left(nmea.indexOf('*'));
    QStringList f_list = line.split(',');

    if (f_list.size() < 17)
        return mpe1;

    mpe1.heading = f_list[1].toDouble();
    mpe1.pitch   = f_list[2].toDouble();
    mpe1.roll    = f_list[3].toDouble();
    mpe1.yaw     = f_list[4].toDouble();

    mpe1.vx = f_list[5].toDouble();  mpe1.vy = f_list[6].toDouble();  mpe1.vz = f_list[7].toDouble();
    mpe1.ax = f_list[8].toDouble();  mpe1.ay = f_list[9].toDouble();  mpe1.az = f_list[10].toDouble();
    mpe1.gx = f_list[11].toDouble();  mpe1.gy = f_list[12].toDouble();

    mpe1.s1 = f_list[13].toInt();
    mpe1.s2 = f_list[14].toInt();
    mpe1.s3 = f_list[15].toInt();
    mpe1.s4 = f_list[16].toInt();

    mpe1.valid = true;   // sentence parsed successfully
    return mpe1;
}


GllData GPS_Listener::m_parseGLL(const QString &nmea)
{
    GllData gll;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 7)
        return gll;

    gll.talker = m_GetConstellationString(f_list[0].mid(1, 2));              // GP / GN

    QTime tmpTime = QTime::fromString(f_list[5], "hhmmss.zzz");
    gll.utc  = tmpTime.addSecs(5 * 3600 + 30 * 60);
    //gll.utc = QTime::fromString(f_list[5], "hhmmss.zzz");
    gll.status = f_list[6].isEmpty() ? 'V' : f_list[6][0];
    gll.mode   = (f_list.size() > 7 && !f_list[7].isEmpty()) ? f_list[7][0] : 'N';

    gll.lat = m_parseLatLon(f_list[1], f_list[2]);
    gll.lon = m_parseLatLon(f_list[3], f_list[4]);

    if (gll.status == 'A')
    {
        gll.valid = true;
    }

    return gll;
}

VtgData GPS_Listener::m_parseVTG(const QString &nmea)
{
    VtgData vtg;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 9)
        return vtg;

    vtg.talker = m_GetConstellationString(f_list[0].mid(1, 2));       // GP / GN
    vtg.courseTrue = f_list[1].toDouble();  // degrees
    vtg.speedKmh   = f_list[7].toDouble();  // km/h
    vtg.mode       = f_list[8].isEmpty() ? 'N' : f_list[8][0];

    vtg.valid = (vtg.mode != 'N');
    return vtg;
}

GsaData GPS_Listener::m_parseGSA(const QString &nmea)
{
    GsaData gsa;

    QString line = nmea.trimmed();
    int star = line.indexOf('*');
    if (star > 0)
        line = line.left(star);

    QStringList f_list = line.split(',');
    if (f_list.size() < 17)
        return gsa;

    gsa.talker  = m_GetConstellationString(f_list[0].mid(1, 2));        // GP / GN
    gsa.mode1   = f_list[1].isEmpty() ? 'A' : f_list[1][0];
    gsa.fixType = f_list[2].toInt();

    // Satellite IDs (fields 3–14)
    for (int i = 3; i <= 14; ++i) {
        if (!f_list[i].isEmpty())
            gsa.satIds.append(f_list[i].toInt());
    }

    gsa.pdop = f_list[15].toDouble();
    gsa.hdop = f_list[16].toDouble();
    gsa.vdop = (f_list.size() > 17) ? f_list[17].toDouble() : 99.99;

    // Optional system ID (newer NMEA)
    if (f_list.size() > 18)
        gsa.systemId = f_list[18].toInt();

    gsa.valid = (gsa.fixType == 3);
    return gsa;
}
