#include "ota_log.h"

OTA_log::OTA_log()
{
}

void OTA_log::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutex mutex;
    mutex.lock();
    QString text;
    switch (type)
    {
        case QtDebugMsg:
            text = QString("Debug: ");
        break;

        case QtWarningMsg:
            text = QString("Warning:");
        break;

        case QtCriticalMsg:
            text = QString("Critical: ");
        break;

        case QtFatalMsg:
            text = QString("Fatal: ");
        break;

        default:
            text = QString("Normal: ");
        break;
    }
    QString context_info = QString("File:(%1) Line :(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy - MM - dd hh : mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1%2%3%4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("./OTA_Log.txt");
    if(!file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        printf("logFile open failed!");
    }
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}
