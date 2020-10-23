#pragma once

#include <string>

#include <QDir>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>
#include <QCryptographicHash>

#include "../thirdParty/nlohmann/json.hpp"



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
            isGood_ = false;
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
        QByteArray num= QCryptographicHash::hash(content.toLocal8Bit(), QCryptographicHash::Md5);
        md5.append(num.toHex());
        return md5;
    }

    QString MD5(const QByteArray& buffer)
    {
        QByteArray num= QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
        QString md5;
        md5.append(num.toHex().toLower());
        return md5;
    }

    bool IsGood()
    {
        return isGood_;
    }

    void ResetState()
    {
        isGood_ = true;
    }

private:

    Q_DISABLE_COPY(MD5Checker)

    bool isGood_;
};


class MD5FolderScanner
{
public:
    MD5FolderScanner(const QString& str)
        :type_(0),
        folder_(str)
    {
        QFileInfo info(folder_);
        if (info.isRelative())
            folder_ = info.absoluteFilePath();
    }

    //怕是没时间写了
    MD5FolderScanner& SetOutputType(int type)
    {
        type_ = type;
        return *this;
    }

    MD5FolderScanner& SetOutputFilename(const QString& filename)
    {
        outPutFilename_ = filename;
        return *this;
    }

    bool Scan()
    {
        MD5Checker checker;
        bool result = ScanFolder(folder_, checker);
        if (!result)
            return false;
    }

private:
    void WriteJson(const char* u8key, const char* u8Value)
    {
        if (0 == type_)
        {
            if (json_.find(u8key) != json_.end())
            {
                qDebug() << "json key already exist,:" << "u8key";
                return;
            }
            json_[u8key] = u8Value;
        }
    }

    //we use this for a known limited depth
    bool ScanFolder(const QString& foler, MD5Checker& checker)
    {
        QFileInfo info(foler);
        if (!info.exists() || !info.isDir())
        {
            qWarning() << "invalid folder path"<< foler;
            return false;
        }

        QDir dir(foler);
        if (dir.entryInfoList().isEmpty())
        {
            qWarning() << "folder info empty, check accessRights" << foler;
            return false;
        }
            
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
                WriteJson(relativePath.toStdString().c_str(), result.toStdString().c_str());
            }
            else
            {
                if (subFileInfo.fileName() == "." || subFileInfo.fileName() == "..")
                    continue;
                qDebug() << "handling Dir :" << subFileInfo.absoluteFilePath();
                ScanFolder(subFileInfo.absoluteFilePath(), checker);
            }
        }
        return true;
    }

    void WriteFile()
    {
        // json_
        QFileInfo f(outPutFilename_);
        QString appFolder = QCoreApplication::applicationDirPath() + "/";

        QString fullPath = appFolder + outPutFilename_;

        QFile targetFile(fullPath);
        if (targetFile.exists())
            QFile::remove(fullPath);
        if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Append))
            return;
        //if(type_)
        targetFile.write(json_.dump().c_str());
    }

private:
    Q_DISABLE_COPY(MD5FolderScanner)

    int type_;
    QString folder_;
    nlohmann::json json_;//or QHash?
    bool useMultiThread_;
    QString outPutFilename_;
};

