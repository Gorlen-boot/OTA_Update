#include <QCoreApplication>
#include <QTextCodec>
#include "mainfunc.h"
#include "ota_log.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(OTA_log::outputMessage);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    mainfunc mm;
    return a.exec();
}
