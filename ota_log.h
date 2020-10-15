#ifndef OTA_LOG_H
#define OTA_LOG_H

//#pragma execution_character_set("utf-8")
#include <QObject>
#include <QMutex>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

class OTA_log
{
public:
    OTA_log();
    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // OTA_LOG_H
