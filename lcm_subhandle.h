#ifndef LCM_SUBHANDLE_H
#define LCM_SUBHANDLE_H

#include <QObject>
#include <lcm/lcm-cpp.hpp>
#include "exlcm/OTA_Requst_t.hpp"
#include "global.h"

class lcm_subHandle : public QObject
{
    Q_OBJECT
public:
    lcm_subHandle(){}
    void handleMessage(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                           const exlcm::OTA_Requst_t *msg);

signals:
    void sub_get(uint8_t res);  //res==1:允许更新   res==2：来自客户端的更新请求
};

#endif // LCM_SUBHANDLE_H
