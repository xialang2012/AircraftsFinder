#ifndef MATHLIB_Header_AD
#define MATHLIB_Header_AD

#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

// stdev
template<typename T>
double getStDev(const std::vector<T> & inVec)
{
	double sum = std::accumulate(std::begin(inVec), std::end(inVec), 0.0);
	double mean = sum / inVec.size(); //mean

	double accum = 0.0;
	std::for_each(std::begin(inVec), std::end(inVec), [&](const double d) {
		accum += (d - mean)*(d - mean);
	});

	double stdev = sqrt(accum / (inVec.size() - 1)); //·½std

	return stdev;
}

// max
template<typename T>
T getMax(const std::vector<T> & inVec)
{
	auto maxPosition = std::max_element(inVec.begin(), inVec.end());
	return inVec[maxPosition - inVec.begin()];
}

// max
template<typename T>
T getMin(const std::vector<T> & inVec)
{
	auto maxPosition = std::min_element(inVec.begin(), inVec.end());
	return inVec[maxPosition - inVec.begin()];
}

// Sobel
template<typename T, typename T1>
void getSobel(const T* value, T1 * sobData, int cols, int rows)
{
	T1 Gx;
	T1 Gy;
	for (int row = 1; row < rows - 1; ++row)
	{
		for (int col = 1; col < cols - 1; ++col)
		{
			Gx = value[(row - 1) * cols + col - 1] + 2 * value[(row - 1)* cols + col] + value[(row - 1) * cols + col + 1]
				- value[(row + 1) * cols + col - 1] - 2 * value[(row + 1)* cols + col] - value[(row + 1) * cols + col + 1];

			Gy = value[(row + 1) * cols + col + 1] + 2 * value[(row)* cols + col + 1] + value[(row - 1) * cols + col + 1]
				- value[(row + 1) * cols + col - 1] - 2 * value[(row)* cols + col - 1] - value[(row - 1) * cols + col - 1];

			sobData[row*cols + col] = abs(Gx) + abs(Gy);
		}
	}
}

// Laplace and Sobel, LS
template<typename T, typename T1>
void getSL(const T* value, T1 * LSData, int cols, int rows)
{
	for (int row = 1; row < rows - 1; ++row)
	{
		for (int col = 1; col < cols - 1; ++col)
		{
			LSData[row*cols + col] = 4 * value[row*cols + col] - (value[(row + 1)*cols + col] + value[(row - 1)*cols + col] + value[row*cols + col + 1]
				+ value[row *cols + col - 1]);
		}
	}

}

// Laplace
template<typename T, typename T1>
void getLaplacian(const T* value, T1 * lapData, int cols, int rows)
{
	for (int row = 1; row < rows - 1; ++row)
	{
		for (int col = 1; col < cols - 1; ++col)
		{
			lapData[row*cols + col] = 4 * value[row*cols + col] - (value[(row + 1)*cols + col] + value[(row - 1)*cols + col] + value[row*cols + col + 1]
				+ value[row *cols + col - 1]);
		}
	}
}


// Find the value in the data that is equal to the condition and get the value of the value that matches the first value
template<typename T, typename T1>
int getEqualValue(const T* inVal, const T& val, const size_t length, std::vector<T1>& tmpIndex)
{
	int count = 0;
	for (size_t i = 0; i < length; ++i)
	{
		if (inVal[i] == val)
		{
			++count;
			tmpIndex.push_back(i);
		}
	}

	return count;
}

// Reset array traversal to a value
template<typename T>
int resetValue(T* inVal, const T& val, const size_t length)
{
	int count = 0;
	for (size_t i = 0; i < length; ++i)
	{
		inVal[i] = val;
	}

	return 0;
}


#endif // !FUNCTIONS_Header_AD
