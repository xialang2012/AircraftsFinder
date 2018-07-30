#include "baseIO.h"
#include "ogrsf_frmts.h"

#ifndef CREATEAIRCRAFTSHP_Header_AD
#define CREATEAIRCRAFTSHP_Header_AD

class HandleShpFile
{
public:
	HandleShpFile(const std::string &outShpFile) : shpName(outShpFile) { initShp(); }

	int addToShp(const std::vector<double> &lats, const std::vector<double> &lons, const std::string &sceneId);
	int closeShp();
	
	~HandleShpFile();

private:
	std::string shpName;
	GDALDriver *poDriver;
	OGRLayer *poLayer;
	OGRSpatialReference* poSpatialRef;
	GDALDataset *poDS;

	int initShp();
};

class CreateLineShp
{
public:
	CreateLineShp(const std::string &outShpFile) : shpName(outShpFile) { initShp(); }

	int addToShp(const std::vector<double> &lats, const std::vector<double> &lons, const std::string &sceneId);
	int closeShp();

	~CreateLineShp();

private:
	std::string shpName;
	GDALDriver *poDriver;
	OGRLayer *poLayer;
	OGRSpatialReference* poSpatialRef;
	GDALDataset *poDS;

	int initShp();
};

// 将多个飞机点合并成一个点
int aircraftToLatLonPoint(std::vector<double> &lons, std::vector<double> &lats, const int &cols,
	const OGRSpatialReference &inRef, const double *adfDstGeoTransform, std::vector<int> &finalAircraftsIndex);

void ImageRowCol2Projection(const double *adfGeoTransform, const int iCol, const int iRow, double &dProjX, double &dProjY);

// zoom aircraft piiel to 30*30
int fillAround(const BYTE* finalAircraft, const UINT16* bandCirrus, BYTE* jpgData, const int& cols, const int & rows);


int getBorder(const UINT16* bandCirrus, const int cols, const int rows, std::vector<int>& borderPoints);

// generate result image
template <typename T>
int genImg(const std::string& jpgFile, const OLIData<T>& dataInfo, const BYTE* jpgData)
{
	char **papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");

	int iWidth = dataInfo.GetWidth();
	int iHeight = dataInfo.GetHeigh();

	GDALAllRegister();
	GDALDriver *hDriver = (GDALDriver *)GDALGetDriverByName("GTiff");
	GDALDataset *hDstDS = (GDALDataset *)GDALCreate(hDriver, jpgFile.c_str(), iWidth, iHeight, 1, GDT_Byte, papszOptions);

	// 写入投影信息
	hDstDS->SetProjection(dataInfo.GetProj().c_str());
	hDstDS->SetGeoTransform(dataInfo.GetGT());
	GDALRasterBand *oBand = hDstDS->GetRasterBand(1);

	//GDALColorInterp GCI_PaletteIndexUsed = GCI_PaletteIndex;
	//oBand->SetColorInterpretation(GCI_PaletteIndexUsed);

	//获取颜色表，设置颜色表  
	GDALColorTable * pColorTable;//颜色表  
	pColorTable = new GDALColorTable;

	GDALColorEntry * pColorEntry = new GDALColorEntry;
	pColorEntry->c1 = 255; //background
	pColorEntry->c2 = 255;
	pColorEntry->c3 = 255;
	pColorEntry->c4 = 0;
	pColorTable->SetColorEntry(255, pColorEntry);

	GDALColorEntry * pColorEntryAir = new GDALColorEntry;
	pColorEntryAir->c1 = 255; //aircraft
	pColorEntryAir->c2 = 0;
	pColorEntryAir->c3 = 0;
	pColorEntryAir->c4 = 0;
	pColorTable->SetColorEntry(1, pColorEntryAir);

	GDALColorEntry * pColorEntryBorder = new GDALColorEntry;
	pColorEntryBorder->c1 = 0; //border
	pColorEntryBorder->c2 = 0;
	pColorEntryBorder->c3 = 0;
	pColorEntryBorder->c4 = 0;
	pColorTable->SetColorEntry(0, pColorEntryBorder);

	oBand->SetColorTable(pColorTable); //给输出图像设置颜色表

	delete pColorEntry;
	delete pColorEntryAir;
	delete pColorEntryBorder;
	delete pColorTable;

	oBand->RasterIO(GF_Write, 0, 0, iWidth, iHeight, (void *)jpgData, iWidth, iHeight, GDT_Byte, 0, 0);

	CSLDestroy(papszOptions);
	GDALClose(hDstDS);

	return 0;
}

#endif