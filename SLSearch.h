#include "baseIO.h"

#ifndef SLSEARCH_Heaer_AD
#define SLSEARCH_Heaer_AD


// 15 * 15 matrix search, the previous version of the algorithm is 11 * 11
const int padLeft = 15;
const int padTop = 15;
const float thresholdLap = 500;
const float threVec[2] = { 10, -100.0 };

template<typename T>
void SLSearch(const OLIData<T>& dataInfo138, const FLOAT32* resultLap, const FLOAT32* resultSL, const int rows, const int cols, BYTE *resultQA)
{
	UINT16* bandCirrus = dataInfo138.GetValue();

	int radiusLen = long(padLeft / 2);
	int xLen = radiusLen * 2 + 1;
	int yLen = radiusLen * 2 + 1;

	BYTE* pointValArr = new BYTE[xLen * yLen];
	BYTE* pointValArrLS = new BYTE[xLen * yLen];
	BYTE* pointValArrCenter = new BYTE[xLen * yLen];
	FLOAT32* pointValArrBorder = new FLOAT32[xLen * yLen];

	std::vector<int> tmpIndex;

	//std::cout << resultQA[cols * 6239 + 7255] << std::endl;	
	std::vector<double> centerLatLon = dataInfo138.GetCenterLatLon();

	// 
	for (int row = padLeft; row < rows - padLeft; ++row)
	{
		for (int col = padLeft; col < cols - padLeft; ++col)
		{

			if (resultQA[cols*row + col] != 1)
			{
				resultQA[cols*row + col] = 0;
				continue;
			}

			// The 1.38 channel value of the current point must be greater than the value of the surrounding four fields.
			int avaiableCount = 0;
			if (bandCirrus[row*cols + col - 1] < bandCirrus[row*cols + col]) ++avaiableCount;
			if (bandCirrus[row*cols + col + 1] < bandCirrus[row*cols + col]) ++avaiableCount;
			if (bandCirrus[(row - 1)*cols + col] < bandCirrus[row*cols + col]) ++avaiableCount;
			if (bandCirrus[(row + 1)*cols + col] < bandCirrus[row*cols + col]) ++avaiableCount;
			if (avaiableCount != 4)
			{
				resultQA[cols*row + col] = 0;
				continue;
			}

			// reset to 0
			int count = 0;
			std::vector<float> ratios;
			resetValue<BYTE>(pointValArr, (BYTE)0, xLen * yLen);
			resetValue<BYTE>(pointValArrLS, (BYTE)0, xLen * yLen);
			resetValue<BYTE>(pointValArrCenter, (BYTE)0, xLen * yLen);
			resetValue<FLOAT32>(pointValArrBorder, (FLOAT32)0, xLen * yLen);

			/* Find based on Sobel Laplacian connection */
			connectionFindSL(col, row, resultSL, radiusLen, cols, threVec, pointValArrCenter, pointValArrLS, pointValArrBorder);

			// Center area four areas at least three points are valid and considered to be valid
			// left and right up and down
			avaiableCount = 0;
			if (pointValArrLS[radiusLen*xLen + radiusLen - 1] == 2) ++avaiableCount;
			if (pointValArrLS[radiusLen*xLen + radiusLen + 1] == 2) ++avaiableCount;
			if (pointValArrLS[(radiusLen - 1)*xLen + radiusLen] == 2) ++avaiableCount;
			if (pointValArrLS[(radiusLen + 1)*xLen + radiusLen] == 2) ++avaiableCount;
			if (avaiableCount <= 2)
			{
				resultQA[cols*row + col] = 0;
				continue;
			}

			count = getEqualValue<BYTE, int>(pointValArrLS, (BYTE)2, xLen * yLen, tmpIndex);
			if (count > 15 || count < 5)
			{
				resultQA[cols*row + col] = 0;
				continue;
			}

			//;  Aspect ratio
			ratios = aspectRatio(pointValArrLS, 2, padLeft, xLen);
			if (ratios[0] <= 0.45)
			{
				resultQA[cols*row + col] = 0;
				continue; //An aspect ratio of less than 1:4 is considered non-aircraft
			}
			//;  area ratio
			if (ratios[1] <= 0.6)
			{
				resultQA[cols*row + col] = 0;
				continue; //Area ratio less than 0.625 is considered non-aircraft
			}

			//;  whether there is a boundary point in the x y direction of the center point.
			avaiableCount = 0;
			int thresBorder = -2500;
			for (int i = 0; i < xLen; ++i)
			{
				int tmpCount = 0;
				if (pointValArrBorder[radiusLen*xLen + i] != 0)
				{
					// Must be less than -1000
					if (pointValArrBorder[(radiusLen - 1)*xLen + i] > -1000 ||
						pointValArrBorder[radiusLen*xLen + i] > -1000 ||
						pointValArrBorder[(radiusLen + 1)*xLen + i] > -1000) continue;
					// And there are 2 less than -2500
					if (pointValArrBorder[(radiusLen - 1)*xLen + i] < thresBorder) ++tmpCount;
					if (pointValArrBorder[radiusLen*xLen + i] < thresBorder) ++tmpCount;
					if (pointValArrBorder[(radiusLen + 1)*xLen + i] < thresBorder) ++tmpCount;
					//if (tmpCount >= 2)++avaiableCount;
					++avaiableCount;
				}
			}
			for (int i = 0; i < yLen; ++i)
			{
				int tmpCount = 0;
				if (pointValArrBorder[i*xLen + radiusLen] != 0)
				{
					// Must be less than -1000
					if (pointValArrBorder[i*xLen + radiusLen - 1] > -1000 ||
						pointValArrBorder[i*xLen + radiusLen] > -1000 ||
						pointValArrBorder[i*xLen + radiusLen + 1] > -1000) continue;
					// And there are 2 less than -2500
					if (pointValArrBorder[i*xLen + radiusLen - 1] < thresBorder) ++tmpCount;
					if (pointValArrBorder[i*xLen + radiusLen] < thresBorder) ++tmpCount;
					if (pointValArrBorder[i*xLen + radiusLen + 1] < thresBorder) ++tmpCount;
					//if (tmpCount >= 2)++avaiableCount;
					++avaiableCount;
				}
			}
			if (avaiableCount <= 1)
			{
				resultQA[cols*row + col] = 0;
				continue;
			}

			//
			std::vector<int> recentanglePixel;
			std::vector<int> recentanglePixelR138;
			for (int tmpRow = row - padTop; tmpRow < row + padTop; ++tmpRow)
			{
				if (tmpRow >(row - 3) && tmpRow < (row + 3)) continue;

				for (int tmpCol = col - padTop; tmpCol < col + padTop; ++tmpCol)
				{
					if (tmpCol >(col - 3) && tmpCol < (col + 3)) continue;

					recentanglePixel.push_back(resultSL[tmpRow*cols + tmpCol]);
					recentanglePixelR138.push_back(bandCirrus[tmpRow*cols + tmpCol]);
				}
			}

			// africa			
			if ((centerLatLon[0] >= -66 && centerLatLon[0] <= 0) && (centerLatLon[1] >= -100 && centerLatLon[1] <= -20))
			{
				// 
				double stDev = getStDev<int>(recentanglePixel);
				// Standard deviation within the elements of the center point 5 to 15 mean ref
				double meanBorderPixelRef138 = std::accumulate(std::begin(recentanglePixelR138), std::end(recentanglePixelR138), 0.0) / recentanglePixelR138.size();
				if (stDev > 500)
				{
					if ((bandCirrus[row*cols + col] - meanBorderPixelRef138) < 1800)
					{
						resultQA[cols*row + col] = 0;
						continue;
					}
				}
			}

			// max point comparision
			int maxRecentangle = getMax<int>(recentanglePixelR138);
			if (maxRecentangle > bandCirrus[row*cols + col])
			{
				resultQA[cols*row + col] = 0;
				continue;
			}
			//std::cout << col << " " << row << " " << bandCirrus[row*cols + col] << std::endl;	
			resultQA[cols*row + col] = 1;
		}
	}

	delete[] pointValArr;
	delete[] pointValArrLS;
	delete[] pointValArrCenter;
	delete[] pointValArrBorder;
}


// Sobel Lapiacian connection find
int connectionFindSL(const int &col, const int &row, const FLOAT32* resultSL, const int &radiusLen, const int &cols, const float *threVec, BYTE* pointValArrCenter, BYTE* pointValArrLS, FLOAT32* pointValArrBorder);

int cal4NerStatusSLNew(const int &x, const int &y, const int &centX, int &centY, const FLOAT32* resultSL, const int &radiusLen, const int& cols, const float *threVec, BYTE* pointValArr, FLOAT32 *pointValArrBorder);

int cal4NerStatusSLCenter(const int &x, const int &y, const int &centX, int &centY, const FLOAT32* resultSL, const int &radiusLen, const int& cols, const float *threVec, BYTE* pointValArr);

// aspect ratio for candinate aircraft 返回面积比和长宽比 vector (iyRate, areaRate)
std::vector<float> aspectRatio(const BYTE* pointValArr, const int& flag, const int& padLeft, const int& iLen);

#endif // !SLSEARCH_Heaer_AD