#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QTextCodec>
#include <QUrl>
#include <QString>
#include <QFile>
#include <QFileInfo>

class http_download :public QObject
{
    Q_OBJECT
public:
    http_download(); //构造函数，要求下载地址，保存位置，进度条（可选）
    virtual ~http_download();
    void getFile(QString url_str, QString pathtoSave); //开始下载
    void startRequest(QUrl url);
    void cancel();

private:
    //=====状态变量=====
    bool HttpError;
    QFile *downloadFile;                //保存目标文件指针
    QUrl downloadUrl;                   //下载地址Url
    QString savePath;                   //文件存放路径
    QString versionCode = "";           //所下载的文件版本号
    QString fileName = "";              //所下载文件名
    QNetworkReply *downloadReply;       //网络应答指针
    QNetworkAccessManager downloadmanager;//网络连接主类指针

signals:
    void error(QNetworkReply::NetworkError); //当错误发生时向外抛出错误信息
    void updateProgress(qint64, qint64); //抛出下载进度(更新进度条)
    void downloadResult(int retCode); //下载结果 0成功 其他失败
    void eventStop();

private slots:
    void downloadReadyRead(); //准备下载，读取数据
    void downloadFinished(); //下载完成
    void downloadError(QNetworkReply::NetworkError); //下载过程中发生错误
    void downloadProgress(qint64, qint64);//下载过程进度条
};

#endif // HTTP_DOWNLOAD_H
