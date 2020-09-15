#ifndef MAINFUNC_H
#define MAINFUNC_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <lcm/lcm-cpp.hpp>
#include "lcm_subhandle.h"
#include "global.h"
#include "ota_download.h"

class mainfunc : public QObject
{
    Q_OBJECT
public:
    mainfunc();
    uint8_t update_check(void);
    int start_update(void);
    int old_backup();
    int new_download(char* url, QString &filename);
    int new_install(QString filepath,QString  install_path);
    int file_crc(QString file_path,QString remote_md5);
    int old_restore(void);
    int update_json(void);

private slots:
    void client_requst_respond(uint8_t res);
    void update_lcmres();

private:
    int timercount;
    int poweron_flag=0;
    QTimer   lcm_spin;
    lcm::LCM ota_lcm_pub;
    lcm::LCM ota_lcm_sub;
    lcm_subHandle lcm_subhandle;
    QFile local_json;
    QFile server_json;
    ota_download mhttp_download;
};

#endif // MAINFUNC_H
