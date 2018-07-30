#include "genShpIMG.h"

// reference http://www.gdal.org/ogr_apitut.html

int HandleShpFile::initShp()
{
	const char *pszDriverName = "ESRI Shapefile";
	//GDALDriver *poDriver;
	GDALAllRegister();
	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == NULL)
	{
		std::cout << "driver not available" << pszDriverName << std::endl;
		return -1;
	}

	//GDALDataset *poDS;
	poDS = poDriver->Create(shpName.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (poDS == NULL)
	{
		std::cout << "Creation of output file failed" << std::endl;
		return -1;
	}

	poSpatialRef = new OGRSpatialReference();
	poSpatialRef->importFromEPSG(4326);
	poLayer = poDS->CreateLayer("Aircrafts", poSpatialRef, wkbPoint, NULL);
	if (poLayer == NULL)
	{
		std::cout << "Layer creation failed" << std::endl;
		return -1;
	}

	OGRFieldDefn oField("sceneId", OFTString);
	oField.SetWidth(32);
	if (poLayer->CreateField(&oField) != OGRERR_NONE)
	{
		std::cout << "Creating Name field failed" << std::endl;
		return -1;
	}

	OGRFieldDefn oField1("aircrafts", OFTInteger);
	oField1.SetWidth(8);
	if (poLayer->CreateField(&oField1) != OGRERR_NONE)
	{
		std::cout << "Creating Name field failed" << std::endl;
		return -1;
	}
	
	return 0;
}

int HandleShpFile::closeShp()
{
	return 0;
}

int HandleShpFile::addToShp(const std::vector<double> &lats, const std::vector<double> &lons, const std::string &sceneId)
{
	for (int i = 0; i < lons.size(); ++i)
	{
		double x = lons[i], y = lats[i];

		OGRFeature *poFeature;
		poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
		poFeature->SetField("sceneId", sceneId.c_str());
		poFeature->SetField("aircrafts", static_cast<int>(lons.size()));
		//OGRLinearRing
		OGRPoint pt;
		pt.setX(x);
		pt.setY(y);
		poFeature->SetGeometry(&pt);
		if (poLayer->CreateFeature(poFeature) != OGRERR_NONE)
		{
			std::cout << "Failed to create feature in shapefile" << std::endl;
			OGRFeature::DestroyFeature(poFeature);
			return -1;
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	return 0;
}

HandleShpFile::~HandleShpFile()
{	
	poSpatialRef->Release();
	GDALClose(poDS);
}



int CreateLineShp::initShp()
{
	const char *pszDriverName = "ESRI Shapefile";
	GDALAllRegister();
	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if (poDriver == NULL)
	{
		std::cout << "driver not available" << pszDriverName << std::endl;
		return -1;
	}

	//GDALDataset *poDS;
	poDS = poDriver->Create(shpName.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (poDS == NULL)
	{
		std::cout << "Creation of output file failed" << std::endl;
		return -1;
	}

	poSpatialRef = new OGRSpatialReference();
	poSpatialRef->importFromEPSG(4326);
	poLayer = poDS->CreateLayer("Aircrafts", poSpatialRef, wkbLineString, NULL);
	if (poLayer == NULL)
	{
		std::cout << "Layer creation failed" << std::endl;
		return -1;
	}

	OGRFieldDefn oField("sceneId", OFTString);
	oField.SetWidth(32);
	if (poLayer->CreateField(&oField) != OGRERR_NONE)
	{
		std::cout << "Creating Name field failed" << std::endl;
		return -1;
	}

	return 0;
}


int CreateLineShp::addToShp(const std::vector<double> &lats, const std::vector<double> &lons, const std::string &sceneId)
{		
	OGRFeature *poFeature;
	poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
	poFeature->SetField("sceneId", sceneId.c_str());
	
	double z = 0;

	if (abs(lons[1] - lons[3]) > 180)
	{
		return 0;

		// left
		OGRLineString ptLeft;
		double x = 0, y = 0;
		for (int i = 0; i < lons.size(); ++i)
		{
			if (i == 1 || i == 2)
			{
				if (lons[i] > 0)
				{
					x = 180;
				}
				else
				{
					y = -180;
				}
				y = lats[i];
			}
			else
			{
				x = lons[i], y = lats[i];
			}
			ptLeft.addPoint(x, y, z); std::cout << "left" << x << y << z << std::endl;
		}
		ptLeft.addPoint(lons[0], lats[0], z);
		poFeature->SetGeometry(&ptLeft);

		// right
		OGRLineString ptRight;
		for (int i = 0; i < lons.size(); ++i)
		{
			if (i == 0 || i == 3)
			{
				if (lons[i] > 0)
				{
					x = 180;
				}
				else
				{
					y = -180;
				}
				y = lats[i];
			}
			else
			{
				x = lons[i], y = lats[i];
			}
			ptRight.addPoint(x, y, z); std::cout << "right" << x << y << z << std::endl;
		}
		ptRight.addPoint(-180, lats[0], z);
		poFeature->SetGeometry(&ptRight);
	}
	else
	{
		OGRLineString pt;
		for (int i = 0; i < lons.size(); ++i)
		{
			double x = lons[i], y = lats[i];
			pt.addPoint(x, y, z);
		}
		pt.addPoint(lons[0], lats[0], z);
		poFeature->SetGeometry(&pt);
	}
	
	if (poLayer->CreateFeature(poFeature) != OGRERR_NONE)
	{
		std::cout << "Failed to create feature in shapefile" << std::endl;
		OGRFeature::DestroyFeature(poFeature);
		return -1;
	}

	OGRFeature::DestroyFeature(poFeature);
	return 0;
}

CreateLineShp::~CreateLineShp()
{

}
int CreateLineShp::closeShp()
{
	poSpatialRef->Release();
	GDALClose(poDS);
	return 0;
}

void getCenterPoint(const UINT16* bandCirrus, const BYTE* resultQA, const int &cols, const int &rows, BYTE *finalAircraft, std::vector<int> &finalAircraftsIndex)
{
	int padLeft = 10;
	int padTop = 10;

	// 
	int maxBandCirrus = 0;
	int rowOneAircraft = 0, colOneAircraft = 0;

	//	
	for (int row = padLeft; row < rows - padLeft; ++row)
	{
		//std::cout << row << std::endl;
		for (int col = padTop; col < cols - padTop; ++col)
		{
			if (resultQA[row * cols + col] == 1.0)
			{
				if (finalAircraft[row * cols + col] != 0) continue;

				maxBandCirrus = 0;

				for (int tmpRow = row - 3; tmpRow < row + 3; ++tmpRow)
				{
					for (int tmpCol = col - 3; tmpCol < col + 3; ++tmpCol)
					{
						if (resultQA[tmpRow * cols + tmpCol] == 1.0)
						{
							finalAircraft[tmpRow * cols + tmpCol] = 2;

							if (bandCirrus[tmpRow * cols + tmpCol] > maxBandCirrus)
							{
								maxBandCirrus = bandCirrus[tmpRow * cols + tmpCol];
								rowOneAircraft = tmpRow;
								colOneAircraft = tmpCol;
							}

						}
					}
				}
				// add to tmpIndex
				finalAircraft[rowOneAircraft*cols + colOneAircraft] = 1;
				finalAircraftsIndex.push_back(rowOneAircraft*cols + colOneAircraft);
			}

		}
	}
}

// 
int aircraftToLatLonPoint(std::vector<double> &lons, std::vector<double> &lats, const int &cols, 
	const OGRSpatialReference &inRef, const double *adfDstGeoTransform, std::vector<int> &finalAircraftsIndex)
{	
	int count = finalAircraftsIndex.size();
	int col, row;
	double dProjX, dProjY;

	OGRSpatialReference* poSpatialRef = new OGRSpatialReference();
	poSpatialRef->importFromEPSG(4326);
	OGRCoordinateTransformation *coordTrans;
	coordTrans = OGRCreateCoordinateTransformation(const_cast<OGRSpatialReference *>(&inRef), const_cast<OGRSpatialReference *>(poSpatialRef));

	int aircraftCount = 0;
	for (int i = 0; i < count; ++i)
	{
		if (finalAircraftsIndex[i] == 0) continue;
		col = finalAircraftsIndex[i] % cols;
		row = floor(finalAircraftsIndex[i] / float(cols));

		// get cordination for the row and col, then convert to geographic location
		++aircraftCount;
		ImageRowCol2Projection(adfDstGeoTransform, col, row, dProjX, dProjY);
		coordTrans->Transform(1, &dProjX, &dProjY);

		lats.push_back(dProjY);
		lons.push_back(dProjX);
	}
	coordTrans->DestroyCT(coordTrans);
	poSpatialRef->Release();

	return aircraftCount;
}


inline void ImageRowCol2Projection(const double *adfGeoTransform, const int iCol, const int iRow, double &dProjX, double &dProjY)
{
	dProjX = adfGeoTransform[0] + adfGeoTransform[1] * iCol + adfGeoTransform[2] * iRow;
	dProjY = adfGeoTransform[3] + adfGeoTransform[4] * iCol + adfGeoTransform[5] * iRow;
}

// expand aircraft pixel for bettter visualization
int fillAround(const BYTE* finalAircraft, const UINT16* bandCirrus, BYTE* jpgData, const int& cols, const int & rows)
{
	int	padLeft = 25;
	int aircraftCount = 0;

	//	
	for (int row = padLeft; row < rows - padLeft; ++row)
	{
		for (int col = padLeft; col < cols - padLeft; ++col)
		{

			// color for aircraft of RGB(255, 0, 0)
			if (finalAircraft[row*cols + col] == 1)
			{
				++aircraftCount;

				//jpgData[i - padLeft:i + padLeft, j - padLeft : j + padLeft] 
				for (int tmpRow = row - padLeft; tmpRow <= row + padLeft; ++tmpRow)
				{
					for (int tmpCol = col - padLeft; tmpCol <= col + padLeft; ++tmpCol)
					{
						jpgData[tmpRow*cols + tmpCol] = 1;
						//std::cout << "f" << std::endl;
					}
				}
			}

			//	borde of image
			int tmpPadLeft = padLeft / 8;
			if ((bandCirrus[row*cols + col] == 0) && (bandCirrus[row*cols + col + 1] != 0))
			{
				//jpgData[i - padLeft / 8:i + padLeft / 8, j - padLeft / 8 : j + padLeft / 8] = 1;
				for (int tmpRow = row - tmpPadLeft; tmpRow <= row + tmpPadLeft; ++tmpRow)
				{
					for (int tmpCol = col - tmpPadLeft; tmpCol <= col + tmpPadLeft; ++tmpCol)
					{
						jpgData[tmpRow*cols + tmpCol] = 0;
					}
				}
			}
			if ((bandCirrus[row*cols + col] != 0) && (bandCirrus[row*cols + col + 1] == 0))
			{
				//jpgData[i - padLeft / 8:i + padLeft / 8, j - padLeft / 8 : j + padLeft / 8] = 1;
				for (int tmpRow = row - tmpPadLeft; tmpRow <= row + tmpPadLeft; ++tmpRow)
				{
					for (int tmpCol = col - tmpPadLeft; tmpCol <= col + tmpPadLeft; ++tmpCol)
					{
						jpgData[tmpRow*cols + tmpCol] = 0;
					}
				}
			}/**/

		}
	}

	return aircraftCount;

}

int getBorder(const UINT16* bandCirrus, const int cols, const int rows, std::vector<int>& borderPoints)
{
	// top bottom
	int rowTop = 0, rowBottom = rows - 1, flag=0;
	// left right
	int colLeft = 0, colRight = cols - 1;

	// top
	for (rowTop = 0; rowTop < rows; ++rowTop)
	{
		if (flag != 0)break;
		for (int col = 0; col < cols; ++col)
		{
			if (bandCirrus[rowTop*cols + col] != 0)
			{
				borderPoints.push_back(rowTop*cols + col);
				//borderX.push_back(col);
				//borderY.push_back(rowTop);
				flag++; break;
			}
		}
	}

	// right
	flag = 0;
	for (colRight = cols - 1; colRight > 0; --colRight)
	{
		if (flag != 0)break;
		for (int row = 0; row < rows; ++row)
		{
			if (bandCirrus[row*cols + colRight] != 0)
			{
				borderPoints.push_back(row*cols + colRight);
				//borderX.push_back(colRight);
				//borderY.push_back(row);
				flag++; break;
			}
		}
	}

	// bottom
	flag = 0;
	for (rowBottom = rows-1; rowBottom > 0; --rowBottom)
	{
		if (flag != 0)break;
		for (int col = 0; col < cols; ++col)
		{
			if (bandCirrus[rowBottom*cols + col] != 0)
			{
				borderPoints.push_back(rowBottom*cols + col);
				//borderX.push_back(col);
				//borderY.push_back(rowBottom);
				flag++; break;
			}
		}
	}


	// left
	flag = 0;
	for (colLeft = 0; colLeft < cols; ++colLeft)
	{
		if (flag != 0)break;
		for (int row = 0; row < rows; ++row)
		{
			if (bandCirrus[row*cols + colLeft] != 0)
			{
				borderPoints.push_back(row*cols + colLeft);
				//borderX.push_back(colLeft);
				//borderY.push_back(row);
				flag++; break;
			}
		}
	}

	return 0;
}
