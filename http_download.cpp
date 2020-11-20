#include "http_download.h"
#include "global.h"
#include <QTimer>
#include <QEventLoop>
#include <unistd.h>

http_download::http_download()
{
    downloadFile = nullptr;
    downloadReply = nullptr;
    bisFinished = false;
    HttpError = false;
}

http_download::~http_download()
{
    cancel();
}

void http_download::getFile(QString url_str, QString pathtoSave)
{
    int index=0;
    QUrl Url(url_str);
    QString filename= Url.fileName();
    index= url_str.lastIndexOf('/');
    QString basepath = url_str.remove(index+1,filename.size());
    QByteArray byte = filename.toUtf8().toPercentEncoding();
    byte = QByteArray(basepath.toStdString().c_str()) + byte;

    QUrl encodeUrl(byte);
    downloadUrl = encodeUrl.fromEncoded(byte);
    savePath = pathtoSave + filename;
    qDebug()<<"下载地址:"<<downloadUrl;

    if(savePath.isEmpty() || downloadUrl.isEmpty())
    {
        qDebug()<<"路径错误";
        return ;
    }
    downloadFile = new QFile(savePath);
    if(!downloadFile->open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug()<<"下载文件打开错误";
        delete downloadFile;
        downloadFile = nullptr;
        return ;
    }
    Global::DownloadOK= false;
    startRequest(downloadUrl); //尝试获取文件 //事件循环，防止在下载没完成前结束对象
}

void http_download::startRequest(QUrl url)
{
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(1000*20);
    timer.setSingleShot(true);
    QNetworkRequest req(url);
    downloadReply = downloadmanager.get(req);
    if(downloadReply)  //连接信号与槽
    {
        connect(downloadReply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
        connect(downloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(this, SIGNAL(eventStop()), &loop, SLOT(quit()));
        connect(&timer,SIGNAL(timeout()),&loop, SLOT(quit()));
        connect(downloadReply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(downloadError(QNetworkReply::NetworkError)));
    }
    timer.start();
    loop.exec();
    if(timer.isActive())
    {
        // 处理响应
        timer.stop();
//        if(downloadReply->error() != QNetworkReply::NoError)
//        {
//            qDebug() << "Error String : " << downloadReply->errorString(); // 错误处理
//        }
//        else
//        {
//            QVariant variant = downloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
//            int nStatusCode = variant.toInt();
//            qDebug() << "Status Code : " << nStatusCode; // 根据状态码做进一步数据处理
//        }
    }
    else
    {
        // 处理超时
        disconnect(downloadReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        downloadReply->abort();
        downloadReply->deleteLater();
        downloadReply= nullptr;
        downloadFile->close();
        downloadFile->remove();
        usleep(100);
        downloadFile->deleteLater();
        downloadFile = nullptr;
        qDebug() << "Timeout";
    }
}

void http_download::cancel()
{
    if(downloadFile)
    {
        downloadFile->close();
        downloadFile->deleteLater();
        downloadFile = nullptr;
    }
    if(downloadReply)
    {
        downloadReply->deleteLater();
        downloadReply = nullptr;
    }
}

void http_download::downloadReadyRead()
{
    if(downloadFile)
    {
        downloadFile->write(downloadReply->readAll());
    }
}

void http_download::downloadFinished()
{
    downloadFile->flush();
    downloadFile->close();

    downloadReply->deleteLater();
    downloadReply = nullptr;
    if(HttpError)
    {
        Global::DownloadOK =false;
    }
    else
    {
        Global::DownloadOK=true;
        if(downloadFile)
        {
            downloadFile->deleteLater();
            downloadFile = nullptr;
        }
    }
    emit eventStop();
}

void http_download::downloadError(QNetworkReply::NetworkError) //下载过程中发生错误
{
    HttpError =true;
    cancel();
}

void http_download::downloadProgress(qint64, qint64)//下载过程进度条
{

}





