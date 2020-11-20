#include "mainfunc.h"
#include "md5_compare.h"
#include <unistd.h>

#define HZ_UPDATE (3600*12)

mainfunc::mainfunc()
{
    timercount=0;
    poweron_flag=1;
    if(!ota_lcm_pub.good())
    {
        qDebug("********************分隔符*********************");
        qDebug("发布进程通讯异常!");
    }
    else
    {
        qDebug("********************分隔符*********************");
        qDebug("发布进程通讯正常!");
    }
    if(!ota_lcm_sub.good())
    {
        qDebug("订阅进程通讯异常!");
    }
    else
    {
        qDebug("订阅进程通讯正常!");
    }
    ota_lcm_sub.subscribe("OTA_RESPONED",&lcm_subHandle::handleMessage, &lcm_subhandle);
    connect(&lcm_subhandle,SIGNAL(sub_get(uint8_t)),this,SLOT(client_requst_respond(uint8_t)));
    connect(&lcm_spin, SIGNAL(timeout()), this, SLOT(update_lcmres()));
    lcm_spin.start(1000);
}

//定时检测与发布请求
void mainfunc::client_requst_respond(uint8_t res)
{
    if(res==1)  //客户端同意更新请求
    {
        int update_flags=0;
        update_flags=old_backup(); //当前文件备份
        if(update_flags==-1)
        {
            qDebug("备份失败");
            return;
        }
        update_flags=start_update();//允许更新
        if(update_flags!=0)
        {
            qDebug("更新失败");
            old_restore();
            return;
        }
        else
        {
            update_json();
            exlcm::OTA_Requst_t reqData;
            reqData.Update_Finished=1;
            ota_lcm_pub.publish("OTA_REQUEST",&reqData);
            qDebug("更新完成");
        }
    }
    if(res==2)  //来自客户端更新请求
    {
        uint8_t res;
        exlcm::OTA_Requst_t reqData;
        res=update_check();
        if(res==0)
        { 
            reqData.module_update_requst=1;
            qDebug()<<"有可用更新，等待客户端响应！";
        }
        else if(res==1 || res==2) //更新失败
        {
            reqData.module_update_respond=2;
            qDebug()<<"更新失败！";
        }
        else if(res==3)
        {  
            reqData.module_update_respond=3;
            qDebug()<<"最新版程序，不需要更新！";
        }
        ota_lcm_pub.publish("OTA_REQUEST",&reqData);
    }
}

void mainfunc::update_lcmres()
{
    uint8_t res=0;
    timercount++;
    ota_lcm_sub.handleTimeout(10);
    if(timercount >= HZ_UPDATE || poweron_flag==1)
    {
        res=update_check();
        if(res==0) //LCM消息发送升级请求
        {
            exlcm::OTA_Requst_t reqData;
            reqData.module_update_requst=1;
            ota_lcm_pub.publish("OTA_REQUEST",&reqData);
            qDebug()<<"有可用更新，等待客户端响应！";
        }
        else if(res==2)
        {
            qDebug("自动更新，json文件解析失败！");
        }
        else if(res==3)
        {
            qDebug("最新版程序，不需要更新！");
        }
        timercount=0;
        poweron_flag=0;
    }
}

uint8_t mainfunc::update_check(void)
{
    QByteArray local_ba;
    QByteArray remote_ba;
    QString m_url = "http://218.240.159.78/v2x_server/v2x_server_json/remote_update.json";

    mhttp_download.getFile(m_url,"./");
    if(!Global::DownloadOK) return 2;

    local_json.setFileName("./config/local_update.json");
    server_json.setFileName("./remote_update.json");
    if(!local_json.exists())
    {
        system("cp ./config/local_init.json ./config/local_update.json");
        return 0;
    }
    if((!local_json.open(QIODevice::ReadOnly)) || (!server_json.open(QIODevice::ReadOnly)))
    {
        qDebug("服务器升级文档打开失败");
        return 2;
    }
    local_ba  = local_json.readAll();
    remote_ba = server_json.readAll();
    local_json.close();
    server_json.close();
    QJsonParseError e_local,e_remote;
    QJsonDocument jdoc_local= QJsonDocument::fromJson(local_ba,&e_local);
    QJsonDocument jdoc_remote= QJsonDocument::fromJson(remote_ba,&e_remote);
    if((e_local.error==QJsonParseError::NoError && !jdoc_local.isNull()) ||     \
            (e_remote.error==QJsonParseError::NoError && !jdoc_remote.isNull()))
    {
        QJsonObject object_local=jdoc_local.object();
        QJsonObject object_remote=jdoc_remote.object();
        QJsonArray  arr_local = object_local.value("UpdateFile").toArray();
        QJsonArray  arr_remote = object_remote.value("UpdateFile").toArray();
        qDebug()<<arr_local.size()<<" "<<arr_remote.size();
        if(arr_local.size() != arr_remote.size()) return 0;
        else
        {
            for(int i=0;i<arr_remote.size();i++)
            {
                uint8_t find_t=0;
                QJsonObject arrobj_remote = arr_remote[i].toObject();
                for(int j=0;j<arr_local.size();j++)
                {
                    QJsonObject arrobj_local = arr_local[i].toObject();
                    if(arrobj_local.value("FileName")==arrobj_remote.value("FileName"))
                    {
                        find_t=1;
                        if(arrobj_local.value("Version") != arrobj_remote.value("Version"))
                        {
                            return 0;
                        }
                        else
                           break;
                    } 
                }
                if(find_t==0)
                {
                    return 0;
                }
            }
            return 3;
        }
    }
    else
    {
         qDebug("升级策略解析失败！");
         return 2;
    }
}

//下载更新文件
//return 0:成功  1:更新文件打开失败  2:下载失败   3:校验失败
int mainfunc::start_update()
{
    QByteArray local_ba;
    QByteArray remote_ba;
    QString download_path;
    QString install_path;
    QString file_name;
    QString md5str;
    local_json.setFileName("./config/local_update.json");
    server_json.setFileName("./remote_update.json");

    if((!local_json.open(QIODevice::ReadOnly)) || (!server_json.open(QIODevice::ReadOnly)))
    {
        qDebug("读升级文档失败!");
        return 2;  //更新失败
    }
    local_ba  = local_json.readAll();
    remote_ba = server_json.readAll();
    local_json.close();
    server_json.close();
    QJsonParseError e_local,e_remote;
    QJsonDocument jdoc_local= QJsonDocument::fromJson(local_ba,&e_local);
    QJsonDocument jdoc_remote= QJsonDocument::fromJson(remote_ba,&e_remote);
    if((e_local.error==QJsonParseError::NoError && !jdoc_local.isNull()) ||     \
            (e_remote.error==QJsonParseError::NoError && !jdoc_remote.isNull()))
    {
        QJsonObject object_local=jdoc_local.object();
        QJsonObject object_remote=jdoc_remote.object();
        QJsonArray  arr_local = object_local.value("UpdateFile").toArray();
        QJsonArray  arr_remote = object_remote.value("UpdateFile").toArray();
        if(1)
        {
            for(int i=0;i<arr_remote.size();i++)
            {
                uint8_t find_t=0;
                QJsonObject arrobj_remote = arr_remote[i].toObject();
                for(int j=0;j<arr_local.size();j++)
                {
                    QJsonObject arrobj_local = arr_local[i].toObject();
                    if(arrobj_local.value("FileName")==arrobj_remote.value("FileName"))
                    {
                        find_t=1;
                        if(arrobj_local.value("Version") != arrobj_remote.value("Version"))
                        {
                            int genxin_flag;
                            install_path=arrobj_remote.value("Target").toString();
                            md5str=arrobj_remote.value("MD5SUM").toString();
                            download_path=arrobj_remote.value("LocalFilePath").toString();
                            genxin_flag=new_download(download_path,file_name);
                            if(genxin_flag!=0)
                            {
                                qDebug()<<"更新过程下载失败! FileName:"<<arrobj_remote.value("FileName").toString();
                                return 1;
                            }
                            genxin_flag=file_crc(file_name,md5str);
                            if(genxin_flag!=0)
                            {
                                qDebug()<<"文件校验失败!";
                                return 3;
                            }
                            genxin_flag=new_install(file_name,install_path);
                            if(genxin_flag!=0)
                            {
                                qDebug()<<"安装失败!";
                                return 3;
                            }
                        }
                        else
                           break;
                    }
                }
                if(find_t==0) //没找到，说明有新增部分
                {
                    int genxin_flag;
                    QByteArray ba;
                    install_path=arrobj_remote.value("Target").toString();
                    md5str=arrobj_remote.value("MD5SUM").toString();
                    download_path=arrobj_remote.value("LocalFilePath").toString();
                    ba.append(download_path);
                    genxin_flag=new_download(ba.data(),file_name);
                    if(genxin_flag!=0)
                    {
                        qDebug()<<" 更新文件下载失败！FileName:"<<arrobj_remote.value("FileName").toString();
                        return 1;
                    }
                    genxin_flag=file_crc(file_name,md5str);
                    if(genxin_flag!=0)
                    {
                        qDebug()<<"更新文件校验失败!"<<arrobj_remote.value("FileName").toString();
                        return 3;
                    }
                    genxin_flag=new_install(file_name,install_path);
                    if(genxin_flag!=0)
                    {
                        qDebug()<<"更新文件安装失败!"<<arrobj_remote.value("FileName").toString();
                        return 3;
                    }
                }
            }
        }
    }
    else
    {
         return 1;
    }
    return 0;
}

//新文件下载和校验
int mainfunc::new_download(QString url , QString &filename)
{
    int index=url.lastIndexOf('/');
    filename = QDir::currentPath()+url.mid(index);
    mhttp_download.getFile(url,"./");
    if(!Global::DownloadOK) return 1;
    return 0;
}

int mainfunc::new_install(QString filepath,QString  install_path)
{
    int res=0;
    bool ok = false;
    int index = install_path.lastIndexOf('/');
    QString path = install_path.mid(0,index);
    QDir tem_path;
    if(tem_path.mkpath(path))
    {
        if(QFile::exists(install_path))
        {
            QFile temff(install_path);
            temff.remove();
        }
        ok = QFile::copy(filepath,install_path);
        if(!ok)
        {
            res= -1;
            qDebug()<<"文件拷贝失败!"<<endl;
        }
    }
    else
    {
        res = -1;
        qDebug()<<"路径创建失败!"<<endl;
    }
    QFile::remove(filepath);
    usleep(1000);
    qDebug()<<"安装路径:"<<path<<endl;
    return res;
}

int mainfunc::file_crc(QString file_path,QString remote_md5)
{
    QByteArray md5_ba;
    QString local_md5;
    unsigned char* tt=nullptr;
    md5_ba=getFileMd5(file_path);
    tt=reinterpret_cast<unsigned char*>(md5_ba.data());
    for(int i=0;i<md5_ba.size();i++)
    {
       local_md5 += QString("%1").arg(tt[i],2,16,QLatin1Char('0'));
    }
    qDebug()<<"本地端 md5:"<<local_md5;
    qDebug()<<"服务器 md5:"<<remote_md5;
    if(remote_md5.compare(local_md5)!=0)
        return 1;
    else
        return 0;
}

int mainfunc::old_backup()
{
    /* 方法一 备份运行目录下文件*/
    int res;
    res=system("rm -rf /home/lzj/PreCrashGUI_Backup");
    usleep(1000);
    res=system("mkdir -p /home/lzj/PreCrashGUI_Backup");
    usleep(1000);
    res=system("cp -rf /home/lzj/PreCrashGUI/* /home/lzj/PreCrashGUI_Backup/");
    usleep(1000);

    /* 方法二 备份json文件下所有项 */
    /* res=system("rm -rf /home/work_backup");
    res=system("mkdir -p /home/work_backup");
    QFile back_file("./config/local_update.json");
    QByteArray back_ba;
    if((!back_file.open(QIODevice::ReadOnly)))
    {
        qDebug("backup read json file failed");
        return 1;
    }
    back_ba  = back_file.readAll();
    back_file.close();

    QJsonParseError e_back;
    QJsonDocument jdoc_back= QJsonDocument::fromJson(back_ba,&e_back);

    if((e_back.error==QJsonParseError::NoError && !jdoc_back.isNull()))
    {
        QJsonObject object_back=jdoc_back.object();
        QJsonArray  arr_back = object_back.value("UpdateFile").toArray();
        if(arr_back.size()<=0)
        {
            res=1;
            qDebug("Json 文档内容错误！");
            return res;
        }
        else
        {
            for(int i=0;i<arr_back.size();i++)
            {
                QByteArray b_ba;
                QJsonObject back_obj=arr_back[i].toObject();
                QString back_path=back_obj.value("Target").toString();
                back_path="cp -f "+back_path+" /home/work_backup/";
                b_ba.append(back_path);
                res=system(b_ba.data());
            }
        }
    } */

    return res;
}

int mainfunc::old_restore(void)
{
    int res;
    qDebug()<<"开始恢复数据！"<<endl;
    res=system("rm -rf /home/lzj/PreCrashGUI");
    usleep(1000);
    res=system("mkdir -p /home/lzj/PreCrashGUI");
    usleep(1000);
    res=system("cp -rf /home/lzj/PreCrashGUI_Backup/* /home/lzj/PreCrashGUI/");
    usleep(1000);
    return (res&0xFF);
}

int mainfunc::update_json(void)
{
    int res;
    qDebug()<<"更新升级文档！"<<endl;
    res=system("rm -f ./config/local_update.json");
    usleep(1000);
    res=system("mv -f ./remote_update.json ./config/local_update.json");
    usleep(1000);
    return res;
}

int mainfunc::waitTimeout(void)
{
    timercount=0;
    while(!Global::DownloadOK)
    {
        timercount++;
        if(timercount>10000) return -1;
        usleep(1000);
    }
    return 0;
}



