#include "SLSearch.h"

int connectionFindSL(const int &i, const int &j, const FLOAT32* resultSL, const int &radiusLen, const int &cols, const float *threVec, BYTE* pointValArrCenter, BYTE* pointValArr, FLOAT32* pointValArrBorder)
{
	//; 2Ϊ��Чֵ, 1Ϊ������ֵ, 0Ϊδ����ֵ.��û��1ֵʱwhile�˳�
	int xLen = radiusLen * 2 + 1;
	int	yLen = xLen;

	//	; ����ȡ��i, j���������ڵ� ��ֵ�� - 100, ����Ϊ�Ƿɻ�, ����Щ����������6��pointValArrȫ��Ϊ2 ����

//		; ��һ���Ƚ��ɻ��� �� ��Χ��ֵ�γɵĺ�Ȧ ֮��ĵ�(������ֵ 10)��ȡ���� ����Ϊ2
	//	; ��ʼ���ĵ���Զ��4
		//;  radiusLen = 7
	pointValArrCenter[radiusLen*xLen + radiusLen] = 2;
	int	centX = i;
	int centY = j;
	//cal4NerStatusSLCenter(radiusLen, radiusLen, centX, centY, resultSL, pointValArrCenter, radiusLen, 10, -100
	cal4NerStatusSLCenter(radiusLen, radiusLen, centX, centY, resultSL, radiusLen, cols, threVec, pointValArrCenter);

	while (true)
	{
		std::vector<int> tmpIndex;
		int count = getEqualValue<BYTE, int>(pointValArrCenter, (BYTE)1, (size_t)xLen * yLen, tmpIndex);

		std::vector<int> tmpIndexCen;
		int countCen = getEqualValue<BYTE, int>(pointValArrCenter, (BYTE)2, (size_t)xLen * yLen, tmpIndexCen);

		//	; ������6��ʱ���˳�
		if (countCen > 12)
		{
			resetValue<BYTE>(pointValArr, (BYTE)2, xLen * yLen);
			return 0;
		}

		if (count == 0)
		{
			break;
		}
		else
		{
			cal4NerStatusSLCenter(tmpIndex[0] % xLen, tmpIndex[0] / yLen, centX, centY, resultSL, radiusLen, cols, threVec, pointValArrCenter);
		}
	}

	//; �ڶ��� ��Lap ��������
	//; ��ʼ����Զ��2
	const float threVecNew[2] = {-100, 10};
	pointValArr[radiusLen*xLen + radiusLen] = 2;
	//	; ��ǰ����г�ʼ��
	centX = i;
	centY = j;
	cal4NerStatusSLNew(radiusLen, radiusLen, centX, centY, resultSL, radiusLen, cols, threVecNew, pointValArr, pointValArrBorder);

	/*for (int tmpI = 0; tmpI < xLen; ++tmpI)
	{
		for (int tmpJ = 0; tmpJ < xLen; ++tmpJ)
		{
			std::cout << static_cast<int>(pointValArr[tmpI*xLen + tmpJ]) << " ";
		}
		std::cout << std::endl;
	}*/

	while (true)
	{
		std::vector<int> tmpIndex;
		int count = getEqualValue<BYTE, int>(pointValArr, (BYTE)1, (size_t)xLen * yLen, tmpIndex);

		if (count == 0)
		{
			break;
		}
		else
		{
			cal4NerStatusSLNew(tmpIndex[0] % xLen, tmpIndex[0] / yLen, centX, centY, resultSL, radiusLen, cols, threVecNew, pointValArr, pointValArrBorder);
		}

	}

	//; ��pointValArrCenter�����Ʒɻ��еĲ���  �滻�� pointValArr
	for (int tmpRow = 0; tmpRow < xLen; ++tmpRow)
	{
		for (int tmpCol = 0; tmpCol < xLen; ++tmpCol)
		{
			if (pointValArrCenter[tmpRow*xLen + tmpCol] == 2)pointValArr[tmpRow*xLen + tmpCol] = 4;
		}
	}

	return 0;
}

int cal4NerStatusSLCenter(const int &x, const int &y, const int &centX, int &centY, const FLOAT32* resultSL, const int &radiusLen, const int& cols, const float *threVec, BYTE* pointValArr)
{
	//; Set the current point to the completed point
	int	xLen = radiusLen * 2 + 1;
	pointValArr[y*xLen + x] = 2;

	float blackThre = threVec[0];
	float whiteThre = threVec[1];

	//; Top left, bottom right;  Note that it is first determined that the black line is not black and the point is judged as the next candidate point 1
	if (x - 1 >= 0)
	{
		if (pointValArr[y*xLen + x - 1] == 0)
		{
			if (resultSL[x - 1 - radiusLen + centX + (y - radiusLen + centY)*cols] > blackThre)
			{
				pointValArr[y*xLen + x - 1] = 3;
			}
			else if (resultSL[x - 1 - radiusLen + centX + (y - radiusLen + centY)*cols] < whiteThre)
			{
				pointValArr[y*xLen + x - 1] = 1;
			}
		}
	}

	if (y - 1 >= 0)
	{
		if (pointValArr[(y - 1)*xLen + x] == 0)
		{
			if (resultSL[x - radiusLen + centX + (y - 1 - radiusLen + centY)*cols] > blackThre)
			{
				pointValArr[(y - 1)*xLen + x] = 3;
			}
			else if (resultSL[x - radiusLen + centX + (y - 1 - radiusLen + centY)*cols] < whiteThre)
			{
				pointValArr[(y - 1)*xLen + x] = 1;
			}
		}
	}

	if (x + 1 <= radiusLen * 2)
	{
		if (pointValArr[y*xLen + x + 1] == 0)
		{
			if (resultSL[x + 1 - radiusLen + centX + (y - radiusLen + centY)*cols] > blackThre)
			{
				pointValArr[y*xLen + x + 1] = 3;
			}

			else if (resultSL[x + 1 - radiusLen + centX + (y - radiusLen + centY)*cols] < whiteThre)
			{
				pointValArr[y*xLen + x + 1] = 1;
			}
		}
	}

	if (y + 1 <= radiusLen * 2)
	{
		if (pointValArr[(y + 1)*xLen + x] == 0)
		{
			if (resultSL[x - radiusLen + centX + (y + 1 - radiusLen + centY)*cols] > blackThre)
			{
				pointValArr[(y + 1)*xLen + x] = 3;
			}
			else if (resultSL[x - radiusLen + centX + (y + 1 - radiusLen + centY)*cols] < whiteThre)
			{
				pointValArr[(y + 1)*xLen + x] = 1;
			}
		}
	}

	return 0;
}

int cal4NerStatusSLNew(const int &x, const int &y, const int &centX, int &centY, const FLOAT32* resultSL, const int &radiusLen, const int& cols, const float *threVec, BYTE* pointValArr, FLOAT32 *pointValArrBorder)
{
	//
	int	xLen = radiusLen * 2 + 1;
	pointValArr[y*xLen + x] = 2;

	float blackThre = threVec[0];
	float whiteThre = threVec[1];

	//
	if (x - 1 >= 0)
	{
		if (pointValArr[y*xLen + x - 1] == 0)
		{
			if (resultSL[x - 1 - radiusLen + centX + (y - radiusLen + centY)*cols] < blackThre)
			{
				pointValArr[y*xLen + x - 1] = 3;
				pointValArrBorder[y*xLen + x - 1] = resultSL[x - 1 - radiusLen + centX + (y - radiusLen + centY)*cols];
			}
			else if (resultSL[x - 1 - radiusLen + centX + (y - radiusLen + centY)*cols] > whiteThre)
			{
				pointValArr[y*xLen + x - 1] = 1;
			}
		}
	}

	if (y - 1 >= 0)
	{
		if (pointValArr[(y - 1)*xLen + x] == 0)
		{
			if (resultSL[x - radiusLen + centX + (y - 1 - radiusLen + centY)*cols] < blackThre)
			{
				pointValArr[(y - 1)*xLen + x] = 3;
				pointValArrBorder[(y - 1)*xLen + x] = resultSL[x - radiusLen + centX + (y - 1 - radiusLen + centY)*cols];
			}
			else if (resultSL[x - radiusLen + centX + (y - 1 - radiusLen + centY)*cols] > whiteThre)
			{
				pointValArr[(y - 1)*xLen + x] = 1;
			}
		}
	}

	if (x + 1 <= radiusLen * 2)
	{
		if (pointValArr[y*xLen + x + 1] == 0)
		{
			if (resultSL[x + 1 - radiusLen + centX + (y - radiusLen + centY)*cols] < blackThre)
			{
				pointValArr[y*xLen + x + 1] = 3;
				pointValArrBorder[y*xLen + x + 1] = resultSL[x + 1 - radiusLen + centX + (y - radiusLen + centY)*cols];
			}
			else if (resultSL[x + 1 - radiusLen + centX + (y - radiusLen + centY)*cols] > whiteThre)
			{
				pointValArr[y*xLen + x + 1] = 1;
			}
		}
	}

	if (y + 1 <= radiusLen * 2)
	{
		if (pointValArr[(y + 1)*xLen + x] == 0)
		{
			if (resultSL[x - radiusLen + centX + (y + 1 - radiusLen + centY)*cols] < blackThre)
			{
				pointValArr[(y + 1)*xLen + x] = 3;
				pointValArrBorder[(y + 1)*xLen + x] = resultSL[x - radiusLen + centX + (y + 1 - radiusLen + centY)*cols];
			}
			else if (resultSL[x - radiusLen + centX + (y + 1 - radiusLen + centY)*cols] > whiteThre)
			{
				pointValArr[(y + 1)*xLen + x] = 1;
			}
		}
	}

	return 0;
}

std::vector<float> aspectRatio(const BYTE* pointValArr, const int& flag, const int& padLeft, const int& xLen)
{
	std::vector<float> ratio;
	float rateXY = 0;

	std::vector<int> tmpIndex;
	int count = getEqualValue<BYTE, int>(pointValArr, (BYTE)2, xLen * xLen, tmpIndex);

	//	; Calculate the maximum and minimum x y of the effective 2 formed edge, 
	// used to determine whether the formed shape meets the requirements
	int	minX = 0;
	int	minY = 0;
	int	maxX = 0;
	int	maxY = 0;

	for (int countI = 0; countI < count; ++countI)
	{
		int currX = tmpIndex[countI] % padLeft;
		int currY = tmpIndex[countI] / padLeft;

		if (minX == 0)
		{
			minX = currX;
			minY = currY;
			maxX = currX;
			maxY = currY;
		}

		if (currX > maxX) maxX = currX;
		if (currY > maxY) maxY = currY;
		if (currX < minX) minX = currX;
		if (currY < minY) minY = currY;

	}

	float lengthX = float(maxX) - minX + 1;
	float lengthY = float(maxY) - minY + 1;
	rateXY = lengthY / lengthX;
	if (rateXY > 1) rateXY = 1 / rateXY;

	ratio.push_back(rateXY);
	ratio.push_back(count / (lengthX*lengthY - 1));

	return ratio;
}
