#pragma once

#include "windows.h"

#include <QFile>
#include <string>
#include <QFileInfo>
#include <QtGlobal>

#include "AppUtility.h"
#include "thirdParty/zlib/UnZip.h"

/*
* 为更新程序写的解压类
* wchar_t和Qt全家桶格格不入
*use like this, if all setting are default
*UnZipper up;
*up.SetResource(src);
*up.UnZip();
*/

class UnZipper
{
	Q_DISABLE_COPY(UnZipper)
public:
	UnZipper() = default;
	~UnZipper()
	{
		CloseZip(hz_);
	}

	int GetFileCount()
	{
		ZIPENTRY ze;
		GetZipItem(hz_, -1, &ze);
		return ze.index;
	}

	bool SetResource(const std::wstring& path)
	{
		srcPath_ = path;
		hz_ = OpenZip(srcPath_.c_str(), nullptr);
		return hz_;
	}

	//fuckkkkkkk
	void SetTargetPath(std::wstring&& path)
	{
		desPath_ = std::move(path);
		if (desPath_.back() == L'\\')
			desPath_ = path.substr(0, path.length() - 1);
		else
			desPath_ = path;
	}

	bool Backup()
	{
		return BackupTo(desPath_);
	}

	bool UnZip()
	{
		return UnZipTo(desPath_);
	}

	void SetBackupPath(const std::wstring& path)
	{
		backupPath_ = path;
	}

	void Recover()
	{

	}

private:
	//备份原始文件到一个目录
	/*
		这个目录名字是backup/$packname
	*/
	bool BackupTo(const std::wstring& path)
	{
		//QFileInfo
	}

	bool UnZipTo(const std::wstring& path)
	{
		if (Backup())
			return false;
		int numitems = GetFileCount();
		ZIPENTRY ze;
		for (int i = 0; i < numitems; i++)
		{
			if (ZR_OK != GetZipItem(hz_, 100, &ze))
			{
				qDebug() << "GetZipItem error when trying to unzip to local path";
				return false;
			}
			if (ze.attr & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			std::wstring filaname = path + ze.name;//ze not begin with seprator
			auto ret = UnzipItem(hz_, i, filaname.c_str());
			if (ZR_OK != ret)
				return false;
		}
		return true;
	}

private:
	std::wstring srcPath_;
	std::wstring desPath_;
	std::wstring backupPath_;
	HZIP hz_;
};

