#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept> 

#include "gdalwarper.h"
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include "cpl_conv.h"

#include "gdal_alg.h"
#include "gdalwarper.h"

#include "MathLib.h"

#ifdef WIN32
#include <io.h>
#elif linux  
#include <unistd.h>  
#include <dirent.h> 
#endif  

typedef short INT16;
typedef unsigned short UINT16;
typedef float FLOAT32;
typedef unsigned char BYTE;

#ifndef BASEIO_Header_AD
#define BASEIO_Header_AD

// common band base class
template<typename T>
class BandBase
{
public:

	BandBase() = delete;

	BandBase(std::string inFile) : fileName(inFile)
	{
		//openBandData();
		hSrcDS = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
		
		//hSrcDS = (GDALDataset *)GDALOpen("1wrwr.tif", GA_ReadOnly);
		if(hSrcDS==NULL)throw std::invalid_argument("bad input file.");
		//std::cout << hSrcDS << std::endl;

		iBand = hSrcDS->GetRasterBand(1);
		width = iBand->GetXSize();
		heigh = iBand->GetYSize();

		eTypeDst = iBand->GetRasterDataType();
		pszProjectionProj = hSrcDS->GetProjectionRef();
		hSrcDS->GetGeoTransform(iGTDst);

		double dProjX = iGTDst[0] + iGTDst[1] * width / 2;
		double dProjY = iGTDst[3] + iGTDst[5] * heigh / 2;

		OGRSpatialReference* poSpatialRef = new OGRSpatialReference();
		poSpatialRef->importFromEPSG(4326);
		OGRCoordinateTransformation *coordTrans;

		OGRSpatialReference *oSRS = new OGRSpatialReference();
		oSRS->SetFromUserInput(pszProjectionProj);

		coordTrans = OGRCreateCoordinateTransformation(oSRS, const_cast<OGRSpatialReference *>(poSpatialRef));
		coordTrans->Transform(1, &dProjX, &dProjY);
		coordTrans->~OGRCoordinateTransformation();

		poSpatialRef->Release();

		oSRS->Release();

		centerLatLon.push_back(dProjY);
		centerLatLon.push_back(dProjX);
	}

	// file name
	std::string fileName;

	// File Name
	std::string GetFileName()const;

	// Geo info
	std::vector<double> GetCenterLatLon()const;

	// band info for OLI band
	int GetWidth()const;  // width
	int GetHeigh()const;  // heigh

	GDALDataType GetDataType()const;  // data type for the data
	double * GetGT()const;            // six parameters
	T * GetValue()const;          // get point of the band
	GDALDataset *GetFileId()const;	  // get gdal file id
	std::string GetProj() const;  // proj


	void openBandData();

	~BandBase();

private:
	int width;
	int heigh;
	GDALDataset *hSrcDS;
	GDALDataType eTypeDst;
	double iGTDst[6];
	T *dateValue;
	const char * pszProjectionProj;
	GDALRasterBand *iBand;

	std::vector<double> centerLatLon;
};


template<typename T>
void BandBase<T>::openBandData()
{
	GDALAllRegister();

	//GDALDataset *hSrcDSB = (GDALDataset *)GDALOpen(fileName.c_str(), GA_ReadOnly);
	//GDALRasterBand *iBandB = hSrcDSB->GetRasterBand(1);
	//GDALDataType eTypeDst = iBandB->GetRasterDataType();

	//int width = iBandB->GetXSize();
	//int heigh = iBandB->GetYSize();

	//double iGTDstB[6];
	//hSrcDSB->GetGeoTransform(iGTDstB);
	dateValue = new T[width*heigh];
	iBand->RasterIO(GF_Read, 0, 0, width, heigh, dateValue, width, heigh, eTypeDst, 0, 0);
	//iBandB->~GDALRasterBand();

	//hSrcDSB->Release();
}

template<typename T>
std::vector<double> BandBase<T>::GetCenterLatLon() const
{
	return centerLatLon;
}

template<typename T>
int BandBase<T>::GetWidth()const
{
	return width;
}

template<typename T>
int BandBase<T>::GetHeigh()const
{
	return heigh;
}

template<typename T>
GDALDataType BandBase<T>::GetDataType()const
{
	return eTypeDst;
}

template<typename T>
double *BandBase<T>::GetGT()const
{
	return const_cast<double*>(iGTDst);
}

template<typename T>
T *BandBase<T>::GetValue()const
{
	return dateValue;
}

template<typename T>
GDALDataset *BandBase<T>::GetFileId()const
{
	return hSrcDS;
}

template<typename T>
std::string BandBase<T>::GetFileName()const
{
	return fileName;
}

template<typename T>
std::string BandBase<T>::GetProj() const
{
	return std::string(pszProjectionProj);
}

template<typename T>
BandBase<T>::~BandBase()
{
	delete[] dateValue;
	GDALClose(hSrcDS);
}


// oli class
template<typename T>
class OLIData : public BandBase<T>
{
public:

	OLIData() = delete;

	OLIData(std::string inFile) : BandBase(inFile)
	{
		initInfoFromMTL();		
	}

	// time for the data
	std::string GetDateYear()const;
	std::string GetDateMonth()const;
	std::string GetDateDay()const;
	std::string GetDateHour()const;
	std::string GetDateMinute()const;
	std::string GetDateSecond()const;

	// Sun Elevation Angle
	double GetSunElevationAngle()const;

	// Path Row
	std::vector<int> GetRowPath()const;

	std::vector<double> const& getCornerLatLon()const;

	~OLIData();

private:

	double elevationAngleofSun;
	std::vector<double> cornerLatLon;

	std::string year;
	std::string month;
	std::string day;
	std::string hour;
	std::string minute;
	std::string second;

	void initInfoFromMTL();
};

template<typename T>
std::string OLIData<T>::GetDateYear()const
{
	return year;
}

template<typename T>
std::string OLIData<T>::GetDateMonth()const
{
	return month;
}

template<typename T>
std::string OLIData<T>::GetDateDay()const
{
	return day;
}

template<typename T>
std::string OLIData<T>::GetDateHour()const
{
	return hour;
}

template<typename T>
std::string OLIData<T>::GetDateMinute()const
{
	return minute;
}

template<typename T>
std::string OLIData<T>::GetDateSecond()const
{
	return second;
}

template<typename T>
void OLIData<T>::initInfoFromMTL()
{
	FilePathTools pathTool;
	std::string inTmpStr = pathTool.GetFileNameNoSuiffix(fileName);
	std::string inMTLFile = pathTool.GetPath(fileName) + "/" + inTmpStr.substr(0, inTmpStr.length()-2) + "MTL.txt";

	if ((_access(inMTLFile.c_str(), 0)) == -1)
	{
		return;
		throw "file not exit: " + inMTLFile;
	}

	std::string lineStr;
	std::ifstream inStramer(inMTLFile);
	if (inStramer) // open it ok
	{

		while (getline(inStramer, lineStr))
		{
			int index = 0;
			if (!lineStr.empty())
			{
				while ((index = lineStr.find(' ', index)) != std::string::npos) lineStr.erase(index, 1);
			}

			if (lineStr.substr(0, 13) == "DATE_ACQUIRED")
			{
				year = lineStr.substr(14, 4);
				month = lineStr.substr(19, 2);
				day = lineStr.substr(22, 2);
			}

			if (lineStr.substr(0, 17) == "SCENE_CENTER_TIME")
			{
				hour = lineStr.substr(19, 2);
				minute = lineStr.substr(22, 2);
				second = lineStr.substr(25, 2);
			}

			if (lineStr.substr(0, 21) == "CORNER_UL_LAT_PRODUCT")
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 9)));
			if (lineStr.substr(0, 21) == "CORNER_UL_LON_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_UR_LAT_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_UR_LON_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_LL_LAT_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_LL_LON_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_LR_LAT_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));
			if (lineStr.substr(0, 21) == "CORNER_LR_LON_PRODUCT") 
				cornerLatLon.push_back(std::stold(lineStr.substr(22, 6)));

			if (lineStr.substr(0, 13) == "SUN_ELEVATION")
			{
				elevationAngleofSun = std::stold(lineStr.substr(14, 4)); 
				break;
			}
		}
	}
	else
	{
		return;
		throw "failed to open file: " + inMTLFile;
	}
}

template<typename T>
std::vector<double> const& OLIData<T>::getCornerLatLon() const
{
	return cornerLatLon;
}

template<typename T>
double OLIData<T>::GetSunElevationAngle() const
{
	return elevationAngleofSun;
}

template<typename T>
std::vector<int> OLIData<T>::GetRowPath() const
{
	FilePathTools pathTool;

	std::vector<int> pathRow;

	std::string inFile = pathTool.GetFileNameNoSuiffix(fileName);
	std::vector<int> rowPath;

	if (inFile.substr(2, 1) != "0")
	{
		// "PC1"; LC81210362017120LGN00_B9.
		pathRow.push_back(std::stoi(inFile.substr(3, 3)));
		pathRow.push_back(std::stoi(inFile.substr(6, 3)));
	}
	else
	{
		// "C1"; LC08_L1TP_121035_20170525_20170525_01_RT
		pathRow.push_back(std::stoi(inFile.substr(10, 3)));
		pathRow.push_back(std::stoi(inFile.substr(13, 3)));
	}

	return rowPath;
}

template<typename T>
OLIData<T>::~OLIData()
{	
}


template<typename T, typename T1>
int saveTiff(std::string outFile, const OLIData<T>& oliIData, T1* data, const GDALDataType& eTypeDst)
{
	int iWidth = oliIData.GetWidth();
	int iHeight = oliIData.GetHeigh();

	GDALAllRegister();
	GDALDriver *hDriver = (GDALDriver *)GDALGetDriverByName("GTiff");
	GDALDataset *hDstDS = (GDALDataset *)GDALCreate(hDriver, outFile.c_str(), iWidth, iHeight, 1, eTypeDst, NULL);

	// write projetion info
	hDstDS->SetProjection(oliIData.GetProj().c_str());
    //hDstDS->SetGeoTransform(oliIData.GetGT());
	GDALRasterBand *oBand = hDstDS->GetRasterBand(1);
	oBand->RasterIO(GF_Write, 0, 0, iWidth, iHeight, (void *)data, iWidth, iHeight, eTypeDst, 0, 0);
	GDALClose(hDstDS);

	return 0;
}

template<typename T, typename T1>
int saveTiffWithColor(std::string outFile, const OLIData<T>& oliData, T1* data, const GDALDataType& eTypeDst)
{
	int iWidth = dataInfo.GetWidth();
	int iHeight = dataInfo.GetHeigh();

	GDALAllRegister();
	GDALDriver *hDriver = (GDALDriver *)GDALGetDriverByName("GTiff");
	GDALDataset *hDstDS = (GDALDataset *)GDALCreate(hDriver, outFile.c_str(), iWidth, iHeight, 1, eTypeDst, NULL);

	GDALRasterBand *oBand = hDstDS->GetRasterBand(1);
	oBand->RasterIO(GF_Write, 0, 0, iWidth, iHeight, (void *)data, iWidth, iHeight, eTypeDst, 0, 0);
	GDALClose(hDstDS);

	return 0;
}


// File name and path tool
class FilePathTools final
{
public:
	FilePathTools() = default;

	// Get file name only contains file name
	std::string GetFileName(const std::string &);

	// Get the file name contains the path but does not include the suffix
	std::string GetFileNameNoSuiffix(const std::string&);

	// Get suffix
	std::string GetSuiffix(const std::string&);

	// Get path
	std::string GetPath(const std::string&);

	// Get all the files in the directory
	int GetFilesFromDir(const std::string inPath, std::vector<std::string> &filesVec, const std::string suffix = "");

	void replaceAll(std::string& str, const std::string& old_value, const std::string& new_value);

	// if the file is exist on the disk
	bool isFileExist(const std::string &);

	~FilePathTools() = default;

private:

};

#endif // !FUNCTIONS_Header_AD

