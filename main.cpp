#include <QCoreApplication>
#include "mainfunc.h"
#include "ota_log.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(OTA_log::outputMessage);
    mainfunc mm;
    return a.exec();
}
