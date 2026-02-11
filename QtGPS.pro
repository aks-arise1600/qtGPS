QT       += core gui positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    GPS_Listener.cpp \
    gsv_form.cpp \
    main.cpp \
    gpswindow.cpp

HEADERS += \
    GPS_Listener.h \
    StructData.h \
    gpswindow.h \
    gsv_form.h

FORMS += \
    gpswindow.ui \
    gsv_form.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

DISTFILES += \
    android/AndroidManifest.xml \
    android/src/org/example/gps/NmeaListener.java

RESOURCES += \
    icons_resources.qrc
