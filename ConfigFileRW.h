#pragma once
#include <QString>
#include "Alime/noncopyable.h"
#include "thirdParty/nlohmann/json.hpp"


class ConfigFileReadWriter: public noncopyable
{
public:
	static ConfigFileReadWriter& Instance();

	/// <summary>
	/// ������־����·��
	/// </summary>
	/// <returns></returns>
	QString GetLogFilePath() const;

	/// <summary>
	/// �����ļ��е�·��
	/// </summary>
	/// <returns></returns>
	QString GetDownloadFolder() const;

	/// <summary>
	/// ���ز���ʱ, �����ļ�ȫ·��
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
	/// ����ļ�����Ϊ�Ժ�׼���ģ��ڽ��������ǻὫ�ļ��������ִ�в�������
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
	/// Ϊ [����] ��ť׼���Ľӿ�
	/// </summary>
	void SetDownloadFolder();

	/// <summary>
	/// �����û�����
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

