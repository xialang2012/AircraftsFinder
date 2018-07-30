#include <algorithm>
#include <numeric>
#include "baseIO.h"
#include "SLSearch.h"
#include "genShpIMG.h"

#ifndef AIRCRAFTDETECTION_Header_AD
#define AIRCRAFTDETECTION_Header_AD

class GenTypes
{
public:
	GenTypes(bool image = false, bool shp = true, bool Sobel = false, bool Lap = false, bool SL = false, bool resultQA = false) :
		Sobel(Sobel), Lap(Lap), SL(SL), resultQA(resultQA), shp(shp), image(image) {};

	bool genSobel()const;
	bool genLap()const;
	bool genSL()const;
	bool genresultQA()const;
	bool genshp()const;
	bool genimage()const;

	void setSobel(bool status = true);
	void setLap(bool status = true);
	void setSL(bool status = true);
	void setresultQA(bool status = true);
	void setshp(bool status = true);
	void setimage(bool status = true);

	~GenTypes();

private:
	bool Sobel;
	bool Lap;
	bool SL;
	bool resultQA;
	bool shp;
	bool image;

};

class HandleInput
{
public:
	HandleInput(GenTypes *genTypes, std::string inPathStr138 = "", std::string inPathStr21 = "", std::string outPathStr = "")
		: genTypes(genTypes), inPathStr138(""), inPathStr21(""), outPathStr("") {}

	int parseArguments(int argc, char *argv[]);

	std::string inPath138()const;
	std::string inPath21()const;
	std::string inPathVapor()const;
	std::string outPath()const;

	void usage()const;
	~HandleInput();

private:
	GenTypes *genTypes;
	std::string inPathStr138;
	std::string inPathStr21;
	std::string inPathStrVapor;
	std::string outPathStr;

};


template<typename T>
void centerEdgeVerify(std::vector<int> &finalAircraftsIndex, const int rows, const int cols, const OLIData<T>& dataInfo, BYTE *finalAircraft)
{
	int topPad = 5;
	int col, row;
	int count = finalAircraftsIndex.size();

	for (int tmpNum = 0; tmpNum < count; ++tmpNum)
	{
		std::vector<int> totalRef;
		if (finalAircraftsIndex[tmpNum] == 0) continue;

		col = finalAircraftsIndex[tmpNum] % cols;
		row = floor(finalAircraftsIndex[tmpNum] / float(cols));

		// get 10*10 pixels around the center point
		for (int tmpRow = row - topPad; tmpRow < row + topPad && tmpRow >= 0; ++tmpRow)
		{
			for (int tmpCol = col - topPad; tmpCol < col + topPad && tmpCol >= 0; ++tmpCol)
			{
				if ((tmpRow == row - 1) || (tmpRow == row) || (tmpRow == row + 1) ||
					(tmpCol == col - 1) || (tmpCol == col) || (tmpCol == col + 1)) continue;
				totalRef.push_back(dataInfo.GetValue()[tmpRow*cols + tmpCol]);
			}
		}
		double sum = std::accumulate(std::begin(totalRef), std::end(totalRef), 0.0);
		double mean = sum / totalRef.size();
		if (dataInfo.GetValue()[row*cols + col] - mean < 200.0)
		{
			finalAircraftsIndex[tmpNum] = 0;
			finalAircraft[row*cols + col] = 0;
		}

		// Cloud Shadow Judgment Only for South America
		// If there is a pixel in the four fields of the maximum value that is less 
		// than the mean value in the range of 10*10, it is considered as a small cloud spot.
		std::vector<double> centerLatLon = dataInfo.GetCenterLatLon();
		if( !((centerLatLon[0] >= -66 && centerLatLon[0] <= 0) && (centerLatLon[1] >= -100 && centerLatLon[1] <= -20)) ||
			((centerLatLon[0] >= -33 && centerLatLon[0] <= -5) && (centerLatLon[1] >= 0 && centerLatLon[1] <= 52)) ||
			((centerLatLon[0] >= 0 && centerLatLon[0] <= 6) && (centerLatLon[1] >= -61 && centerLatLon[1] <= -47))
			)
		{
			// The ratio of the four points of the central point to the mean of the 10*10 pixels > 0.02 is considered valid
			// left and right up and down
			int avaiableCount = 0;
			if ((static_cast<double>(dataInfo.GetValue()[row*cols + col - 1]) / mean) > 1.025) ++avaiableCount;
			if ((static_cast<double>(dataInfo.GetValue()[row*cols + col + 1]) / mean) > 1.025) ++avaiableCount;
			if ((static_cast<double>(dataInfo.GetValue()[(row - 1)*cols + col]) / mean) > 1.025) ++avaiableCount;
			if ((static_cast<double>(dataInfo.GetValue()[(row + 1)*cols + col]) / mean) > 1.025) ++avaiableCount;
			if (avaiableCount <= 1)
			{
				finalAircraftsIndex[tmpNum] = 0;
				finalAircraft[row*cols + col] = 0;
			}

			avaiableCount = 0;
			if (dataInfo.GetValue()[row*cols + col - 1] < mean || dataInfo.GetValue()[row*cols + col + 1] < mean ||
				dataInfo.GetValue()[(row - 1)*cols + col] < mean || dataInfo.GetValue()[(row + 1)*cols + col] < mean)
			{
				finalAircraftsIndex[tmpNum] = 0;
				finalAircraft[row*cols + col] = 0;
			}
		}

	}

}

// 2.1 micro-meter channel to enhance and remove false alarm caused by low vapor content or high altitude
template<typename T>
void enhanceBy21Channel(std::vector<int> &finalAircraftsIndex, const int rows, const int cols, const OLIData<T>& dataInfo, BYTE *finalAircraft)
{
	int topPad = 7;
	int col, row;
	int count = finalAircraftsIndex.size();

	for (int tmpNum = 0; tmpNum < count; ++tmpNum)
	{
		std::vector<int> totalRef, totalRefBak;
		col = finalAircraftsIndex[tmpNum] % cols;
		row = floor(finalAircraftsIndex[tmpNum] / float(cols));
		
		// get 10*10 pixels around the center point
		for (int tmpRow = row - topPad; tmpRow < row + topPad && tmpRow >= 0; ++tmpRow)
		{
			for (int tmpCol = col - topPad; tmpCol < col + topPad && tmpCol >= 0; ++tmpCol)
			{
				totalRef.push_back( dataInfo.GetValue()[tmpRow*cols + tmpCol] );
			}
		}

		// histogram
		std::sort(totalRef.begin(), totalRef.end());
		totalRefBak = totalRef;
		totalRef.erase(std::unique(totalRef.begin(), totalRef.end()), totalRef.end());

		std::vector<double> countRef; 
		double pixelsCount = static_cast<double>(totalRefBak.size());
		int threshold = 0;
		for each (int ref in totalRef)
		{
			countRef.push_back(std::count(totalRefBak.begin(), totalRefBak.end(), ref));

			// Get the value of position 0.95
			if (static_cast<double>(std::accumulate(countRef.begin(), countRef.end(), 0)) / pixelsCount > 0.95)
			{
				threshold = ref;
				break;
			}
		}

		//int tmpPos = static_cast<int>(totalRef.size() * 0.9);
		//if (tmpPos == 0) continue;

		if (dataInfo.GetValue()[row*cols + col] > threshold)
		{
			//std::cout << row << " " << col << std::endl;
			finalAircraftsIndex[tmpNum] = 0;
			finalAircraft[row*cols + col] = 0;
		}
	}

}

template<typename T>
double getCenterPointVapor(const std::string& inPathVapor, const OLIData<T> &dataInfo138)
{
	// LC81210362017120LGN00_B9
	// C:\\Users\\xialang2012\\Desktop\\aircraftDetection\\vapor/2017_04_12_00.tif"
	// C:\\Users\\xialang2012\\Desktop\\aircraftDetection\\vapor
	std::string inVaporFile = inPathVapor + "/" + dataInfo138.GetDateYear() + "_" + dataInfo138.GetDateMonth() + "_" + dataInfo138.GetDateDay() + "_06.tif";

#ifdef WIN32
	if ((_access(inVaporFile.c_str(), 0)) == -1)
	{
		return -1;
	}
#elif linux
	if ((access(inVaporFile.c_str(), 0)) == -1)
	{
		return -1;
	}
#endif

	std::vector<double> vapor;

	BandBase<short> dataInfoVapor(inVaporFile);
	dataInfoVapor.openBandData();
	double *adfGeoTransform = dataInfoVapor.GetGT();

	// center point
	double dProjX = dataInfo138.GetCenterLatLon()[1];
	double dProjY = dataInfo138.GetCenterLatLon()[0];

	double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
	double dCol = 0.0, dRow = 0.0;
	dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) - adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5;
	dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) - adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;

	int col = int(dCol);
	int row = int(dRow);
	if (col >= 0 && col < dataInfoVapor.GetWidth() && row >0 && row < dataInfoVapor.GetHeigh())
	{
		if (dataInfoVapor.GetValue()[dataInfoVapor.GetWidth() * row + col] < 30000)
		{
			vapor.push_back(5.0);
		}
		else
		{
			vapor.push_back((dataInfoVapor.GetValue()[dataInfoVapor.GetWidth() * row + col] * 0.0013314611669646014 - 43.67868070046324)*0.1);
		}
	}

	// corner point
	for (int i = 0; i < dataInfo138.getCornerLatLon().size(); i = i + 2)
	{
		dProjX = dataInfo138.getCornerLatLon()[i + 1];
		dProjY = dataInfo138.getCornerLatLon()[i];

		double dTemp = adfGeoTransform[1] * adfGeoTransform[5] - adfGeoTransform[2] * adfGeoTransform[4];
		double dCol = 0.0, dRow = 0.0;
		dCol = (adfGeoTransform[5] * (dProjX - adfGeoTransform[0]) - adfGeoTransform[2] * (dProjY - adfGeoTransform[3])) / dTemp + 0.5;
		dRow = (adfGeoTransform[1] * (dProjY - adfGeoTransform[3]) - adfGeoTransform[4] * (dProjX - adfGeoTransform[0])) / dTemp + 0.5;

		int col = int(dCol);
		int row = int(dRow);
		if (col >= 0 && col < dataInfoVapor.GetWidth() && row >0 && row < dataInfoVapor.GetHeigh())
		{
			vapor.push_back((dataInfoVapor.GetValue()[dataInfoVapor.GetWidth() * row + col] * 0.0013314611669646014 + 43.67868070046324)*0.1);
		}
	}

	if (vapor.size() == 0) return -1;

	return getMin<double>(vapor);
}


// get center point of the aircraft
void getCenterPoint(const UINT16* bandCirrus, const BYTE* resultQA, const int &cols, const int &rows, BYTE *finalAircraft, std::vector<int> &finalAircraftsIndex);

// process one file to find aircraft in it
int processOneSwath(std::ofstream &outf, const std::string &inFile, CreateLineShp& cLineShp, HandleShpFile & shpHandle, FilePathTools &pathTool, const GenTypes &genTypes, const std::string &outPath, const std::string &inPath21 = "", const std::string &inPathVapor = "");

// find 8 pixels
int validate8Pixels(const FLOAT32 *resultLap, const int &col, const int &row, const int &cols, const FLOAT32 &thresholdLap);

// find candinate aircraft
int findCandidateAircraft(const UINT16 *bandRef, const FLOAT32 *resultLap, const int &padLeft, const int &padTop, const FLOAT32 &thresholdLap, const int& cols, const int& rows, BYTE *resultQA);

#endif