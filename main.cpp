#include "gpswindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QPermissions>

void requestLocationPermission()
{
#ifdef Q_OS_ANDROID
    QLocationPermission permission;
    permission.setAccuracy(QLocationPermission::Precise);
    permission.setAvailability(QLocationPermission::WhenInUse);

    // checkPermission is non-static
    auto status = qApp->checkPermission(permission);

    if (status == Qt::PermissionStatus::Granted) {
        qDebug() << "Location permission already granted";
        return;
    }

    // requestPermission is non-static
    qApp->requestPermission(
        permission,
        [](const QPermission &perm) {
            if (perm.status() == Qt::PermissionStatus::Granted)
                qDebug() << "Location permission granted";
            else
                qDebug() << "Location permission denied";
        }
        );
#endif
}



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    requestLocationPermission();

    gpsWindow w;
    w.showFullScreen();


    return app.exec();
}
