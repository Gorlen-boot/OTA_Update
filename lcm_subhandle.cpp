#include "lcm_subhandle.h"

void lcm_subHandle::handleMessage(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                       const exlcm::OTA_Requst_t *msg)
{
    if(msg->client_update_respond==1)
    {
       emit sub_get(1);
    }
    if(msg->client_update_requst==1)
    {
       emit sub_get(2);
    }
    printf("Comm Channel is %s",chan.c_str());
    printf("Raw data len = %d",rbuf->data_size);
}

