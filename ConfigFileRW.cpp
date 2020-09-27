#include "ConfigFileRW.h"
#include "AppUtility.h"

#include <fstream>
#include <QMessageBox>
#include <QApplication>


ConfigFileReadWriter& ConfigFileReadWriter::Instance()
{
	static ConfigFileReadWriter rw;
	return rw;
}

ConfigFileReadWriter::ConfigFileReadWriter()
{
	Init();
}

ConfigFileReadWriter::~ConfigFileReadWriter()
{
	SaveToFile();
}

QString ConfigFileReadWriter::GetLogFilePath() const
{
	return logFilePath_;
}

QString ConfigFileReadWriter::GetDownloadFolder() const
{
	return downloadDir_;
}

QString ConfigFileReadWriter::GetUrlOfUpdateInfoFile() const
{
	return updateInfoUrl_;
}

QString ConfigFileReadWriter::GetUrlOfPkgRootFolder() const
{
	return pkgRootFolder_;
}

size_t ConfigFileReadWriter::GetLogLevel() const
{
	return logLever_;
}

size_t ConfigFileReadWriter::GetLogFileRollSize() const
{
	return logFileRollSize_;
}

size_t ConfigFileReadWriter::GetMaxTaskNumber() const
{
	return maxTaskNum_;
}

void ConfigFileReadWriter::SetDownloadFolder()
{
	//fix me, ����д������������Ҫ����ô����
}

void ConfigFileReadWriter::Init()
{
	try
	{
		std::string folder = GetExeFolder();
		std::string filePath= folder + "config.json";
		std::ifstream ifs(filePath);
		ifs >> json_;
		
		QString folderU8=QApplication::applicationDirPath()+"/";
		downloadDir_ = folderU8 +json_["downloadDir"].get<std::string>().c_str();
		logFilePath_= folderU8 + json_["logFilePath"].get<std::string>().c_str();
		updateInfoUrl_= json_["updateInfoUrl"].get<std::string>().c_str();
		logLever_ = ToLogLevel(json_["logLever"].get<std::string>());
		maxTaskNum_ = json_["maxTaskNum"].get<size_t>();
		logFileRollSize_= json_["logFileRollSize"].get<size_t>();
		pkgRootFolder_= json_["pkgRootFolder"].get<std::string>().c_str();
		fixPackFolder_ = pkgRootFolder_+json_["fixPackFolder"].get<std::string>().c_str();
		updatePackFolder_= pkgRootFolder_+json_["updatePackFolder"].get<std::string>().c_str();
	}
	catch (...)
	{
		ShowWarningBox(u8"��������", u8"���������ļ�����", u8"ȷ��");
		std::abort();
	}
}

void ConfigFileReadWriter::SaveToFile()
{
	auto ctn=json_.dump();
	//fix me
}

QString ConfigFileReadWriter::GetUrlOfFixPackFolder() const
{
	return fixPackFolder_;
}

QString ConfigFileReadWriter::GetUrlOfUpdatePackFolder() const
{
	return updatePackFolder_;
}