#include <algorithm>
#include "baseIO.h"


void FilePathTools::replaceAll(std::string& str, const std::string& old_value, const std::string& new_value)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
	{
		if ((pos = str.find(old_value, pos)) != std::string::npos)
		{
			str.replace(pos, old_value.length(), new_value);
		}
		else
		{
			break;
		}
	}
}

std::string FilePathTools::GetFileName(const std::string &inFile)
{
	std::string tmpStr = inFile;
	this->replaceAll(tmpStr, "\\", "/");
	std::size_t pos = tmpStr.find_last_of('/');
	return tmpStr.substr(pos + 1);
}

std::string FilePathTools::GetFileNameNoSuiffix(const std::string &inFile)
{
	std::string tmpStr = inFile;
	this->replaceAll(tmpStr, "\\", "/");

	std::string outNameTmp;
	outNameTmp = GetFileName(tmpStr);
	std::size_t pos = outNameTmp.find_last_of('.');
	return outNameTmp.substr(0, pos);
}

std::string FilePathTools::GetSuiffix(const std::string &inFile)
{
	std::string tmpStr = inFile;
	this->replaceAll(tmpStr, "\\", "/");

	std::string outNameTmp;
	outNameTmp = GetFileName(tmpStr);
	std::size_t pos = outNameTmp.find_last_of('.');
	return outNameTmp.substr(pos + 1);
}

std::string FilePathTools::GetPath(const std::string &inFile)
{
	std::string tmpStr = inFile;
	this->replaceAll(tmpStr, "\\", "/");

	std::string outName;
	std::size_t pos = tmpStr.find_last_of('/');
	return tmpStr.substr(0, pos + 1);
}

int FilePathTools::GetFilesFromDir(const std::string inPath, std::vector<std::string> &filesVec, const std::string suffixIn)
{
	std::string tmpPath = inPath;
	this->replaceAll(tmpPath, "\\", "/");

	std::string suffix = suffixIn;
	std::transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);

#ifdef WIN32

	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(tmpPath).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFilesFromDir(p.assign(tmpPath).append("/").append(fileinfo.name), filesVec, suffix);
			}
			else
			{
				std::string tmpFileSuffix = GetSuiffix(fileinfo.name);
				std::transform(tmpFileSuffix.begin(), tmpFileSuffix.end(), tmpFileSuffix.begin(), tolower);
				if (suffix == "" || tmpFileSuffix == suffix)
					filesVec.push_back(p.assign(tmpPath).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

#elif linux
	DIR *dir;
	struct dirent *ptr;
	char base[1000];

	if ((dir = opendir(inPath.c_str())) == NULL)
	{
		std::cout << "Open dir error..."<std::endl;
		return 0;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir  
		{
			continue;
		}
		else if (ptr->d_type == 8)    ///file  
		{
			std::string tmpFileSuffix = GetSuiffix(ptr->d_name);
			std::transform(tmpFileSuffix.begin(), tmpFileSuffix.end(), tmpFileSuffix.begin(), tolower);
			if (tmpFileSuffix == suffix) filesVec.push_back(inPath + '/' + ptr->d_name);
		}
		else if (ptr->d_type == 10)    ///link file
		{
			continue;
		}
		else if (ptr->d_type == 4)    ///dir  
		{
			continue;
		}
	}
	closedir(dir);
#endif

	return 0;
}


