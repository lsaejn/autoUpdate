#pragma once
#include <QString>
#include "Alime/noncopyable.h"
#include "thirdParty/nlohmann/json.hpp"


class ConfigFileReadWriter: public noncopyable
{
public:
	static ConfigFileReadWriter& Instance();

	/// <summary>
	/// 返回日志保存路径
	/// </summary>
	/// <returns></returns>
	QString GetLogFilePath() const;

	/// <summary>
	/// 下载文件夹的路径
	/// </summary>
	/// <returns></returns>
	QString GetDownloadFolder() const;

	/// <summary>
	/// 本地测试时, 本地文件全路径
	/// </summary>
	/// <returns></returns>
	QString GetLocalPackInfoPath() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	QString GetUrlOfUpdateInfoFile() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	QString GetUrlOfPkgRootFolder() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	QString GetUrlOfFixPackFolder() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	QString GetUrlOfUpdatePackFolder() const;

	/// <summary>
	/// 这个文件夹是为以后准备的，在将来，我们会将文件存在这里。执行差异下载
	/// </summary>
	/// <returns></returns>
	QString GetUrlOfIntegralImageFilesFolder() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	size_t GetLogLevel() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	size_t GetLogFileRollSize() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	size_t GetMaxTaskNumber() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool IsLocalPackFileInfoOn() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool IsSilentInstallationOn() const;

	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool IsDatFileEnabled() const;

	/// <summary>
	/// 为 [设置] 按钮准备的接口
	/// </summary>
	void SetDownloadFolder();

	/// <summary>
	/// 保存用户配置
	/// </summary>
	void SaveToFile();

private:
	ConfigFileReadWriter();
	~ConfigFileReadWriter();

	void Init();

private:
	size_t logLever_;
	size_t maxTaskNum_;
	size_t logFileRollSize_;

	bool useDatFile_ = true;
	bool useLocalPackFile_ = false;
	bool useSilentInstallation_ = false;
	
	QString logFilePath_;
	QString downloadDir_;
	QString updateInfoUrl_;
	QString pkgRootFolder_;
	QString fixPackFolder_;
	QString updatePackFolder_;
	QString integralImageFilesFolder_;
	QString localPackInfoName_;
	
	nlohmann::json json_;
};

