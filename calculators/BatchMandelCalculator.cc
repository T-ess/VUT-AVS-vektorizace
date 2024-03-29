/**
 * @file BatchMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date DATE
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	tile = 64;
	data = (int *)(aligned_alloc(64, height * width * sizeof(int)));
	xData = (float *)(aligned_alloc(64, tile * sizeof(float)));
	yData = (float *)(aligned_alloc(64, tile * sizeof(float)));
}

BatchMandelCalculator::~BatchMandelCalculator() {
	free(data);
	free(xData);
	free(yData);
	data = NULL;
	xData = NULL;
	yData = NULL;
}

int * BatchMandelCalculator::calculateMandelbrot () {
	int *pdata = data;
	int start = 0;
	for (int i = 0; i < height/2; i++) {
		float y = y_start + i * dy; // current imaginary value
		float *xNew = xData;
		float *yNew = yData;
		#pragma omp simd aligned(pdata: 64)
		for (int w = 0; w < width; w++) {
			pdata[i*width+w] = limit;
		}
		for (int tiles = 0; tiles < width/tile; tiles++) {
			start = tile * tiles;
			#pragma omp simd aligned(xNew, yNew: 64)
			for (int t = start; t < start + tile; t++) {
				xNew[t-start] = x_start + t * dx; // current real value
				yNew[t-start] = y;
			}
			int cnt = tile;
			for (int it = 0; it < limit && cnt != 0; ++it) {
				cnt = 0;
				#pragma omp simd early_exit reduction(+:cnt) aligned(pdata, xNew, yNew: 64)
				for (int j = start; j < start + tile; j++) {
					float x = x_start + j * dx; // current real value
					float r2 = xNew[j-start] * xNew[j-start];
					float i2 = yNew[j-start] * yNew[j-start];
					bool is_limit;
					pdata[i*width+j] == limit ? is_limit = true : is_limit = false;
					pdata[i*width+j] = (is_limit && r2 + i2 > 4.0f) ? it : pdata[i*width+j];
					cnt += is_limit;
					yNew[j-start] = 2.0f * xNew[j-start] * yNew[j-start] + y;
					xNew[j-start] = r2 - i2 + x;
				}
			}
		}
		std::memcpy(&pdata[(height-i-1)*width], &pdata[i*width], width*(sizeof(int)));
	}
	return data;
}