#include "FolderCompare.h"
#include "../AppUtility.h"
#include "../ConfigFileRW.h"

bool ResourceCompare::PkpmMD5File::Parse()
{
	return true;
}

bool ResourceCompare::PkpmMD5File::GetFileContent()
{
	auto url = ConfigFileReadWriter::Instance().GetUrlOfIntegralImageFilesFolder();
	//show text
	return true;
}

ResourceCompare::ResourceCompare()
	:rootPath_(GetApplicationDirPath()),
	filesSkiped_(NULL),
	filesUnModified_(NULL),
	filesToDownload_(NULL),
	scanner_(rootPath_)
{
	//ShowProgressBar();
	Init();
}

void ResourceCompare::ShowProgressBar()
{
	dialog_ = new QProgressDialog();
	dialog_->setModal(true);
	dialog_->show();
}

void ResourceCompare::Init()
{
	if (parser_.GetFileContent())
	{
		parser_.Parse();
	}
	else
	{
		//fix me
	}
	//isStateGood_
}

bool ResourceCompare::Scan()
{
	return scanner_.Scan();
}

QString ResourceCompare::RootPath()
{
	return rootPath_;
}

void ResourceCompare::SetRootPath(const QString& localPath)
{
	rootPath_ = localPath;
}

void ResourceCompare::SetFileVector(FileVec* skipped, FileVec* same, FileVec* toDown)
{
	filesSkiped_ = skipped;
	filesUnModified_ = same;
	filesToDownload_ = toDown;
}

void ResourceCompare::CreateCompareResult()
{

}

void ResourceCompare::SetTargetVersion(const QString& version)
{

}

void ResourceCompare::WriteFile(const QString& filePath)
{

}
