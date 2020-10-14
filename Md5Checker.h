#pragma once

#include <string>

#include <QDir>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <QCryptographicHash>

#include "thirdParty/nlohmann/json.hpp"



class MD5Checker
{
public:
    MD5Checker()
        :isGood_(true)
    {
    }

    QString MD5(const QString& filePath)
    {
        QFile localFile(filePath);

        if (!localFile.open(QFile::ReadOnly))
        {
            qDebug() << "file open error.";
            return "";
        }



        quint64 totalBytes = 0;
        quint64 bytesReaded = 0;
        quint64 bytesLeft = 0;

        const quint64 bufferSize = 1024 * 4;//should be 512n
        QByteArray buffer;
        buffer.reserve(bufferSize);

        totalBytes = localFile.size();
        bytesLeft = totalBytes;

        QCryptographicHash ch(QCryptographicHash::Md5);
        while (bytesLeft > 0)
        {
            buffer = localFile.read(qMin(bytesLeft, bufferSize));
            ch.addData(buffer);
            bytesReaded += buffer.length();
            bytesLeft -= buffer.length();
            buffer.resize(0);
        }

        localFile.close();
        QByteArray num = ch.result();
        QString md5;
        md5.append(num.toHex().toLower());
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

    QString MD5(const QByteArray& buffer)
    {

        QByteArray num;
        num = QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
        QString md5;
        md5.append(num.toHex().toLower());
        return md5;
    }

    void ResetState()
    {
        isGood_ = true;
    }

private:
    bool isGood_;
};


class MD5Scanner
{
public:
    MD5Scanner(const QString& str)
        :type_(0),
        folder_(str)
    {
        QFileInfo info(folder_);
        if (info.isRelative())
            folder_ = info.absoluteFilePath();
    }

    MD5Scanner& SetOutputType(int type)
    {
        type_ = type;
        return *this;
    }

    MD5Scanner& SetOutputFilename(const QString& filename)
    {
        outPutFilename_ = filename;
        return *this;
    }

    bool Scan()
    {
        MD5Checker checker;
        // json_
        QFileInfo f(outPutFilename_);
        QString appFolder = QCoreApplication::applicationDirPath() + "/";

        QString fullPath = appFolder + outPutFilename_;

        QFile targetFile(fullPath);
        if (targetFile.exists())
            QFile::remove(fullPath);

        bool result = ScanFolder(folder_, checker);
        targetFile.open(QIODevice::WriteOnly | QIODevice::Append);
        targetFile.write(json_.dump().c_str());
        //qDebug()<<
        return false;
    }

private:
    void WriteFile(const char* u8key, const char* u8Value)
    {
        if (0 == type_)
        {
            if (json_.find(u8key) != json_.end())
            {
                //break here
                getchar();
            }
            json_[u8key] = u8Value;
        }
    }

    //we use this for a known depth
    bool ScanFolder(const QString& foler, MD5Checker& checker)
    {
        QFileInfo info(foler);
        if (!info.exists() || !info.isDir())
        {
            qWarning() << "invalid folder path";
            return false;
        }

        QDir dir(foler);
        if (dir.entryInfoList().isEmpty())
            return false;

        foreach(QFileInfo subFileInfo, dir.entryInfoList())
        {
            if (subFileInfo.isFile())
            {
                auto fullPath = subFileInfo.absoluteFilePath();
                auto result = checker.MD5(fullPath);
                QDir root(folder_);
                auto relativePath = "\\" + root.relativeFilePath(fullPath);
                relativePath.replace('/', "\\");
                qDebug() << "handling File :" << relativePath;
                WriteFile(relativePath.toStdString().c_str(), result.toStdString().c_str());
            }
            else
            {
                if (subFileInfo.fileName() == "." || subFileInfo.fileName() == "..")
                    continue;
                qDebug() << "handling Dir" << subFileInfo.absoluteFilePath();
                ScanFolder(subFileInfo.absoluteFilePath(), checker);
            }
        }

        return true;
    }

private:
    int type_;
    QString folder_;
    nlohmann::json json_;
    QString outPutFilename_;
};

