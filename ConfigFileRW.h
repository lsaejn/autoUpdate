#pragma once
#include <QString>
#include "thirdParty/nlohmann/json.hpp"

class ConfigFileReadWriter
{
public:
	static ConfigFileReadWriter& Instance();
	QString GetLogFilePath() const;
	QString GetDownloadFolder() const;
	QString GetUrlOfUpdateInfoFile() const;
	QString GetUrlOfPkgRootFolder() const;
	QString GetUrlOfFixPackFolder() const;
	QString GetUrlOfUpdatePackFolder() const;
	QString GetUrlOfIntegralImageFilesFolder() const;

	size_t GetLogLevel() const;
	size_t GetLogFileRollSize() const;
	size_t GetMaxTaskNumber() const;

	void SetDownloadFolder();

private:
	ConfigFileReadWriter();
	~ConfigFileReadWriter();
	ConfigFileReadWriter(const ConfigFileReadWriter&)=delete;
	void operator=(const ConfigFileReadWriter&) = delete;

	void Init();
	void SaveToFile();
private:
	size_t logLever_;
	size_t maxTaskNum_;
	size_t logFileRollSize_;
	QString logFilePath_;
	QString downloadDir_;
	QString updateInfoUrl_;
	QString pkgRootFolder_;
	QString fixPackFolder_;
	QString updatePackFolder_;
	QString integralImageFilesFolder_;
	nlohmann::json json_;
};

