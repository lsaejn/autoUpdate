#pragma once
#include <string>

#include <QCryptographicHash>
#include <QString>
#include <QFile>
#include <QDebug>

class MD5Checker
{
public:
	QString MD5(const char* filePath)
	{
        QFile localFile(filePath);

        if (!localFile.open(QFile::ReadOnly))
        {
            qDebug() << "file open error.";
            return "";
        }

        QCryptographicHash ch(QCryptographicHash::Md5);

        quint64 totalBytes = 0;
        quint64 bytesReaded = 0;
        quint64 bytesLeft = 0;

        const quint64 bufferSize = 1024 * 4;//should be 512n
        QByteArray buffer;
        buffer.reserve(bufferSize);

        totalBytes = localFile.size();
        bytesLeft = totalBytes;

        while (bytesLeft>0)
        {
            buffer = localFile.read(qMin(bytesLeft, bufferSize));
            ch.addData(buffer);
            bytesReaded += buffer.length();
            bytesLeft -= buffer.length();
            buffer.resize(0);
        }

        localFile.close();
        QByteArray md5 = ch.result();
        return md5;
	}

	QString MD5(const char* buffer)
	{
		QString md5;
		QString content(buffer);
		QByteArray num;
		num = QCryptographicHash::hash(content.toLocal8Bit(), QCryptographicHash::Md5);
		md5.append(num.toHex());
		return md5;
	}

private:
	bool isGood_;
};
