#ifndef MD5_COMPARE_H
#define MD5_COMPARE_H

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>

QByteArray getFileMd5(QString filePath)
{
    QFile localFile(filePath);
    if(!localFile.open(QFile::ReadOnly))
    {
        qDebug() << "Get MD5 failed!"<<endl;
        return 0;
    }
    QCryptographicHash ch(QCryptographicHash::Md5);
    qint64 totalBytes = 0;
    qint64 bytesWritten = 0;
    qint64 bytesToWrite = 0;
    qint64 loadSize = 1024;
    QByteArray buf;
    totalBytes = localFile.size();
    bytesToWrite = totalBytes;
    while (1)
    {
        if(bytesToWrite > 0)
        {
            buf = localFile.read(qMin(bytesToWrite, loadSize));
            ch.addData(buf);
            bytesWritten += buf.length();
            bytesToWrite -= buf.length();
            buf.resize(0);
        }
        else
        {
            break;
        }
        if(bytesWritten == totalBytes)
        {
            break;
        }
    }
    localFile.close();
    QByteArray md5 = ch.result();
    return md5;
}

QByteArray getFileMd5_2(QString filePath)
{
   QFile theFile(filePath);
   theFile.open(QIODevice::ReadOnly);
   QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
   theFile.close();
   return ba;
}

#endif // MD5_COMPARE_H
