/**
 * @file LineMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	data = (int *)(malloc(height * width * sizeof(int)));
	xData = (float *)(malloc(width * sizeof(float)));
	yData = (float *)(malloc(width * sizeof(float)));
}

LineMandelCalculator::~LineMandelCalculator() {
	free(data);
	free(xData);
	free(yData);
	data = NULL;
	xData = NULL;
	yData = NULL;
}


int *LineMandelCalculator::calculateMandelbrot()
{
	int *pdata = data;
	for (int i = 0; i < height; i++) {
		float y = y_start + i * dy; // current imaginary value
		float *xNew = xData;
		float *yNew = yData;
		for (int w = 0; w < width; w++) {
			pdata[i*width+w] = limit;
			xNew[w] = x_start + w * dx; // current real value;
			yNew[w] = y;
		}
		int cnt = width;
		for (int it = 0; it < limit && cnt != 0; ++it) {
			cnt = 0;
			#pragma omp simd reduction(+:cnt)
            for (int j = 0; j < width; j++) {
                float x = x_start + j * dx; // current real value
                float r2 = xNew[j] * xNew[j];
                float i2 = yNew[j] * yNew[j];
				bool is_limit;
				int res = limit;
				pdata[i*width+j] == limit ? is_limit = true : is_limit = false;
                pdata[i*width+j] = (is_limit && r2 + i2 > 4.0f) ? it : pdata[i*width+j];
				cnt += is_limit;
				// pdata[i*width+j] = res;
				yNew[j] = 2.0f * xNew[j] * yNew[j] + y;
                xNew[j] = r2 - i2 + x;
            }
		}
	}
	return data;
}
