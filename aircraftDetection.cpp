// aircraftDetection.cpp : Defines the entry point for the console application.
//

#include "aircraftDetection.h"

int processOneSwath(std::ofstream &outf, const std::string &inFile, CreateLineShp& cLineShp, HandleShpFile & shpHandle, FilePathTools &pathTool, const GenTypes &genTypes, const std::string &outPath, const std::string &inPath21, const std::string &inPathVapor)
{
	try
	{
		OLIData<UINT16> dataInfo138(inFile);

		if (atoi(dataInfo138.GetDateHour().c_str()) < 8 || atoi(dataInfo138.GetDateHour().c_str()) > 10) return 0;		
		if (atoi(dataInfo138.GetDateHour().c_str()) == 9 && atoi(dataInfo138.GetDateMinute().c_str()) > 30) return 0;

		if (dataInfo138.GetSunElevationAngle() < 5)
		{
			std::cout << "Sun elevation angle is less than 5 deg, continue" << inFile << std::endl;
			return 0;
		}

		std::vector<double> centerLatLon = dataInfo138.GetCenterLatLon();
		//if (!((centerLatLon[0] >= -66 && centerLatLon[0] <= 0) && (centerLatLon[1] >= -100 && centerLatLon[1] <= -20)))return 0;
		//if (!((centerLatLon[0] >= -33 && centerLatLon[0] <= -5) && (centerLatLon[1] >= 0 && centerLatLon[1] <= 52)))return 0;
		//if (!((centerLatLon[0] >= 0 && centerLatLon[0] <= 6) && (centerLatLon[1] >= -61 && centerLatLon[1] <= -47)))return 0;

		dataInfo138.openBandData();
		UINT16	* bandCirrus = dataInfo138.GetValue();

		int rows = dataInfo138.GetHeigh();
		int cols = dataInfo138.GetWidth();

		// test for create border 
		std::vector<double> borderX, borderY;
		std::vector<int> borderPoints;
		getBorder(bandCirrus, cols, rows, borderPoints);

		OGRSpatialReference *oSRSi = new OGRSpatialReference();
		oSRSi->SetFromUserInput(dataInfo138.GetProj().c_str());		

		int countTT = aircraftToLatLonPoint(borderX, borderY, cols, *oSRSi, dataInfo138.GetGT(), borderPoints);

		// vector border of the image
		cLineShp.addToShp(borderY, borderX, pathTool.GetFileName(inFile));
		oSRSi->Release();

		std::string outFile;
		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_bandCirrus.tif";

		FLOAT32* resultLap = new FLOAT32[rows*cols];
		FLOAT32* resultSob = new FLOAT32[rows*cols];
		FLOAT32* resultSL = new FLOAT32[rows*cols];

		//std::cout << "calculate Laplacian sobel and LS..." << std::endl;
		getLaplacian<UINT16, FLOAT32>(bandCirrus, resultLap, cols, rows);
		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_resultLap.tif";
		if (genTypes.genLap()) saveTiff<UINT16, FLOAT32>(outFile, dataInfo138, resultLap, GDT_Float32);

		getSobel<UINT16, FLOAT32>(bandCirrus, resultSob, cols, rows);
		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_resultSob.tif";
		if (genTypes.genSobel()) saveTiff<UINT16, FLOAT32>(outFile, dataInfo138, resultSob, GDT_Float32);

		getSL<FLOAT32, FLOAT32>(resultSob, resultSL, cols, rows);
		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_resultSL.tif";
		if (genTypes.genSL()) saveTiff<UINT16, FLOAT32>(outFile, dataInfo138, resultSL, GDT_Float32);

		delete[] resultSob;

		std::cout << "calculate candidate aircrafts..." << std::endl;
		BYTE* resultQA = new BYTE[rows*cols]();
		findCandidateAircraft(bandCirrus, resultLap, padLeft * 2 + 1, padTop * 2 + 1, thresholdLap, cols, rows, resultQA);
		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_resultCandidate.tif";

		// sobel laplacian search
		std::cout << "calculate SL.." << std::endl;
		SLSearch<UINT16>(dataInfo138, resultLap, resultSL, rows, cols, resultQA);
		delete[] resultLap;
		delete[] resultSL;

		outFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_resultQA.tif";
		if (genTypes.genresultQA())  saveTiff<UINT16, BYTE>(outFile, dataInfo138, resultQA, GDT_Byte);


		// get center point of the aircraft
		std::vector<int> finalAircraftsIndex;
		BYTE *finalAircraft = new BYTE[cols*rows];
		getCenterPoint(bandCirrus, resultQA, cols, rows, finalAircraft, finalAircraftsIndex);

		// 2.1 micro-meter channel to enhance and remove false alarm caused by low vapor content or high altitude	
		if (inPath21 != "")
		{
			std::cout << "2.1 micro-meter enhance..." << std::endl;
			std::string channel21File = inPath21 + "/" + pathTool.GetFileNameNoSuiffix(inFile).substr(0, pathTool.GetFileNameNoSuiffix(inFile).length() - 1) + "7.tif";
#ifdef WIN32
			if ((_access(channel21File.c_str(), 0)) != -1)
			{
				double vapor = getCenterPointVapor(inPathVapor, dataInfo138);
				std::cout << vapor << std::endl;
				if (vapor < 0.8)
				{
					OLIData<UINT16> dataInfo21(channel21File);
					dataInfo21.openBandData();
					enhanceBy21Channel<UINT16>(finalAircraftsIndex, rows, cols, dataInfo21, finalAircraft);
				}
			}
			else
			{
				std::cout << "warning: no matched file for " << pathTool.GetFileName(inFile) << std::endl;
			}
#elif linux
			if ((access(channel21File.c_str(), 0)) != -1)
			{
				double vapor = getCenterPointVapor(inPathVapor, dataInfo138);
				if (vapor > 0.0 && vapor < 1.0)
				{
					OLIData<UINT16> dataInfo21(channel21File);
					dataInfo21.openBandData();
					enhanceBy21Channel<UINT16>(finalAircraftsIndex, rows, cols, dataInfo21, finalAircraft);
					//dataInfo21.DesAll();
				}
			}
			else
			{
				std::cout << "warning: no matched file for " << pathTool.GetFileName(inFile) << std::endl;
			}
#endif
		}

		// create shp file for the aircrafts
		std::vector<double> lons, lats;
		OGRSpatialReference *oSRS = new OGRSpatialReference();
		oSRS->SetFromUserInput(dataInfo138.GetProj().c_str());
		int aircraftCount = aircraftToLatLonPoint(lons, lats, cols, *oSRS, dataInfo138.GetGT(), finalAircraftsIndex);
		shpHandle.addToShp(lats, lons, pathTool.GetFileName(inFile));
		
		oSRS->Release();
		delete[] resultQA;
		// create image file for the aircrafts
		if (genTypes.genimage())
		{
			std::string jpgFile = outPath + "/" + pathTool.GetFileNameNoSuiffix(inFile) + "_browse.tif";
			BYTE* jpgData = new BYTE[cols*rows];
			resetValue<BYTE>(jpgData, (BYTE)255, cols * rows);  // make the background color of the jpg is white 
			fillAround(finalAircraft, bandCirrus, jpgData, cols, rows);
			genImg<UINT16>(jpgFile, dataInfo138, jpgData);/**/
			delete[] jpgData;
		}

		std::cout << "find aircrafts: " << aircraftCount << std::endl;
		if(aircraftCount > 0) outf << inFile << std::endl;

		delete[] finalAircraft;

		return 0;
	}
	catch (std::invalid_argument& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
}

int validate8Pixels(const FLOAT32 *resultLap, const int &col, const int &row, const int &cols, const FLOAT32 &thresholdLap)
{
	int count = 0;
	int countTotal = 0;

	//
	int	minX = 0;
	int	minY = 0;
	int	maxX = 0;
	int	maxY = 0;

	for (int rowTmp = row - 2; rowTmp <= row + 2; ++rowTmp)
	{
		for (int colTmp = col - 2; colTmp <= col + 2; ++colTmp)
		{
			if (resultLap[rowTmp*cols + colTmp] > thresholdLap)
			{
				countTotal = countTotal + 1;

				if (minX == 0)
				{
					minX = colTmp;
					minY = rowTmp;
					maxX = colTmp;
					maxY = rowTmp;
					continue;
				}

				if (colTmp > maxX) maxX = colTmp;
				if (colTmp < minX)  minX = colTmp;

				if (rowTmp > maxY) maxY = rowTmp;
				if (rowTmp < minY) minY = rowTmp;
			}
		}
	}
	if ((maxX - minX + 1 >= 4) || (maxY - minY + 1 >= 4))
	{
		count = 0;
	}
	else
	{
		count = countTotal;
	}

	return count;
}

int findCandidateAircraft(const UINT16 *bandRef, const FLOAT32 *resultLap, const int &padLeft, const int &padTop, const FLOAT32 &thresholdLap, const int& cols, const int& rows, BYTE *resultQA)
{
	//std::cout << resultLap[3000*cols + 3000] << std::endl;
	int count = 0;

	for (int row = padTop; row < rows - padTop; ++row)
	{
		for (int col = padLeft; col < cols - padLeft; ++col)
		{
			/*if (resultLap[row*cols + col] != 0)
			{
			std::cout << row << " " << col << " " << resultLap[row*cols + col] << std::endl;
			}*/
			if (resultLap[row*cols + col] < thresholdLap || bandRef[row*cols + col] < 5200) continue;

			int tmpCount = 0;
			double meanRef = 0.0;
			// 
			/*for (int tmpRow = row - padTop; tmpRow < row + padTop; ++tmpRow)
			{
				for (int tmpCol = col - padTop; tmpCol < col + padTop; ++tmpCol)
				{
					if (resultLap[tmpRow*cols + tmpCol] > thresholdLap) ++tmpCount;
					meanRef += bandRef[tmpRow*cols + tmpCol] / padLeft / padTop;
				}
			}*/

			count = validate8Pixels(resultLap, col, row, cols, thresholdLap);
			
			if (0 != count)  resultQA[row*cols + col] = 1;
			
			/*if (meanRef > 8000.0)
			{
				if (tmpCount == count)	resultQA[row*cols + col] = 1;
			}
			else
			{
				if (0 != count)  resultQA[row*cols + col] = 1;
			}*/
			//std::cout << resultQA[row*cols + col] << std::endl;
			//if(count != 0)std::cout << row << " " << col << " " << resultLap[row*cols + col] << " " << tmpCount << " " << count << std::endl;
		}
	}

	return 0;
}



bool GenTypes::genSobel()const
{
	return Sobel;
}

bool GenTypes::genLap()const
{
	return Lap;
}

bool GenTypes::genSL()const
{
	return SL;
}

bool GenTypes::genresultQA()const
{
	return resultQA;
}

bool GenTypes::genshp()const
{
	return shp;
}

bool GenTypes::genimage()const
{
	return image;
}

void GenTypes::setSobel(bool status)
{
	Sobel = status;
}

void GenTypes::setLap(bool status)
{
	Lap = status;
}

void GenTypes::setSL(bool status)
{
	SL = status;
}

void GenTypes::setresultQA(bool status)
{
	resultQA = status;
}

void GenTypes::setshp(bool status)
{
	shp = status;
}

void GenTypes::setimage(bool status)
{
	image = status;
}

GenTypes::~GenTypes()
{
}


int HandleInput::parseArguments(int argc, char *argv[])
{
	if (argc == 1)
	{
		usage();
		return -1;
	}

	for (int argIndex = 1; argIndex < argc; ++argIndex)
	{
		std::string argument = std::string(argv[argIndex]);
		if (argument == "-help")
		{
			usage();
			return -1;
		} 
		else if (argument == "-inPath1.38")
		{
			++argIndex;
			if (argIndex >= argc)
			{
				std::cout << "more parameters is expected" << std::endl;
				return -1;
			}
			inPathStr138 = std::string(argv[argIndex]);
		}
		else if (argument == "-inPath2.1")
		{
			++argIndex;
			if (argIndex >= argc)
			{
				std::cout << "more parameters is expected" << std::endl;
				return -1;
			}
			inPathStr21 = std::string(argv[argIndex]);
		}
		else if (argument == "-inPathVapor")
		{
			++argIndex;
			if (argIndex >= argc)
			{
				std::cout << "more parameters is expected" << std::endl;
				return -1;
			}
			inPathStrVapor = std::string(argv[argIndex]);
		}
		else if (argument == "-outPath")
		{
			++argIndex;
			if (argIndex >= argc)
			{
				std::cout << "more parameters is expected" << std::endl;
				return -1;
			}
			outPathStr = std::string(argv[argIndex]);
		}
		else if (argument == "-genShp")
		{
			genTypes->setshp();
		}
		else if (argument == "-genImg")
		{
			genTypes->setimage();
		}
		else if (argument == "-genSobel")
		{
			genTypes->setSobel();
		}
		else if (argument == "-genLap")
		{
			genTypes->setLap();
		}
		else if (argument == "-genSL")
		{
			genTypes->setSL();
		}
		else if (argument == "-genResult")
		{
			genTypes->setresultQA();
		}
		else
		{
			usage();
			return -1;
		}
	}

	if (inPathStr138 == "")
	{
		std::cout << "inPath is required" << std::endl;
		return -1;
	}

	if (outPathStr == "")
	{
		std::cout << "outPath is required" << std::endl;
		return -1;
	}

	return 0;
}


std::string HandleInput::inPath138() const
{
	return inPathStr138;
}

std::string HandleInput::inPath21()const
{
	return inPathStr21;
}

std::string HandleInput::inPathVapor()const
{
	return inPathStrVapor;
}

std::string HandleInput::outPath()const
{
	return outPathStr;
}

void HandleInput::usage()const
{
	std::cout << "Usage:" << std::endl;
	std::cout << "      -inPath1.38 (), -outPath (), -inPath2.1 (), -genImg, -genSobel, -genLap, -genSL, -genResult, -help" << std::endl;
	std::cout << "      -inPath1.38 is the input path which includes 1.38 micro-meter tiff files, this parameter is necessary" << std::endl;
	std::cout << "      -outPath is the output path which is used to save all the result files" << std::endl;

	std::cout << "      --inPath2.1 is the input path which includes 2.1 micro-meter tiff files" << std::endl;
	std::cout << "      --inPathVapor is the input path which includes vapor tiff files" << std::endl;
	std::cout << "      -genImg is used to indicate if the result image file will be product and wrote to disk, the default format of the result file is TIFF" << std::endl;
	std::cout << "      -genSobel is used to indicate if the sobel filter file will be wrote to disk, the default format of the result file is TIFF" << std::endl;
	std::cout << "      -genLap is used to indicate if the Laplacian filter file will be wrote to disk, the default format of the result file is TIFF" << std::endl;
	std::cout << "      -genSL is used to indicate if the sobel and Laplacian filter file will be wrote to disk, the default format of the result file is TIFF" << std::endl;
	std::cout << "      -genResult is used to indicate if the result file will be wrote to disk, the default format of the result file is TIFF" << std::endl;

	std::cout << "      -help for help" << std::endl;

}

HandleInput::~HandleInput()
{
	
}