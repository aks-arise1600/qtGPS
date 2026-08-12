#include "device_info.h"

Device_Info::Device_Info() {}


QJniObject Device_Info::m_getActivity()
{
    return QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative",
        "activity",
        "()Landroid/app/Activity;"
        );
}

QString Device_Info::m_androidBoard()
{
#ifdef Q_OS_ANDROID
    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getBoard",
            "()Ljava/lang/String;"
            );
    return obj.toString();
#else
    return QString();
#endif
}
QString Device_Info::m_androidProduct()
{
#ifdef Q_OS_ANDROID
    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getProduct",
            "()Ljava/lang/String;"
            );
    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_androidManufacturer()
{
#ifdef Q_OS_ANDROID
    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getManufacturer",
            "()Ljava/lang/String;"
            );
    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_androidModel()
{
#ifdef Q_OS_ANDROID
    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getModel",
            "()Ljava/lang/String;"
            );
    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_androidId()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getAndroidId",
            "(Landroid/content/Context;)Ljava/lang/String;",
            activity.object<jobject>()
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_androidDeviceId()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getDeviceId",
            "(Landroid/content/Context;)Ljava/lang/String;",
            activity.object<jobject>()
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_getBrand()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getBrand",
            "()Ljava/lang/String;"
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_getDevice()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getDevice",
            "()Ljava/lang/String;"
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_getHardware()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getHardware",
            "()Ljava/lang/String;"
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_getAndroidVersion()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getAndroidVersion",
            "()Ljava/lang/String;"
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_getIMEI()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return QString();

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "getIMEI",
            "(Landroid/content/Context;)Ljava/lang/String;",
            activity.object<jobject>()
            );

    return obj.toString();
#else
    return QString();
#endif
}

QString Device_Info::m_printSimInfo()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return "";

    QJniObject obj =
        QJniObject::callStaticObjectMethod(
            "org/example/gps/DeviceInfo",
            "printSimInfo",
            "(Landroid/content/Context;)Ljava/lang/String;",
            activity.object<jobject>()
            );

    return obj.toString();
#else
    return "";
#endif
}

void Device_Info::m_requestPhonePermission()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = m_getActivity();
    if (!activity.isValid())
        return ;

    QJniObject::callStaticMethod<void>(
        "org/example/gps/DeviceInfo",
        "requestPhonePermission",
        "(Landroid/app/Activity;)V",
        activity.object<jobject>());

    return ;
#else
    return ;
#endif
}
