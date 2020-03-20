#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "gdal.h"
#include "cpl_conv.h"
#include "cpl_string.h"

/*
*   Header file:
*   - Includes
*   - General information
*   - Compilation instructions
*   - Constants
*   - Structured datatype definitions
*   - Function prototypes / forward declarations
*/

/*
*   Bathymetric surface tools was originally written in 2017/2018 to be a part of my master's thesis
*
*   Author:                 Topi Filppula
*   Latest update:          30.03.2019
*
*   Originally written for Unix (Mac OS), possible later versions might or might not be cross-platform.
*
*   Depends on GDAL (Geospatial Data Abstraction Library, see www.gdal.org). GDAL uses MIT/X -type license.
*
*   Compilation instructions:
*   1. Make sure GDAL is installed 
*   2. Link GDAL on compilation
*   3. Optimize and check on compilation
*
*   Compiling:
*   1. Use either Make and the included makefile or 
*   2. Compile manually for example like:
*
*   gcc -g -O3 -march=native -Wall -Wextra -Wfloat-equal -Werror -std=gnu11 -o bathytools *.c -lgdal
*/


// Global constants for floating point comparisons and boolean values:
#define EPSILON     0.00001
#define TRUE        1
#define FALSE       0


// Structured datatype to hold bathymetric surface:
struct FloatSurface {
    char *inputfp;          // Original file path
    char *projection;       // CRS information in WKT
    double *geotransform;   // Georeferencing parameters
    float **array;          // Data array (2D, float**)
    double nodata;          // Source file nodata value
    int rows;               // Number of rows
    int cols;               // Number of columns
};

// Structured datatype to hold the coin:
struct Coin {
    int radius;             // Radius
    int diameter;           // Diameter
    char **array;           // Array (boolean 2D, char**)
};


// Control functions: (main.c)
int main(int argc, const char *argv[]);
void rollingCoinSmoothing(void);
void laplacianSmoothing(void);
void testCoins(void);
void clearScreen(void);
int intInput(const int lower, const int upper, const char *text);

// File input and memory management functions: (inputandmemory.c)
struct FloatSurface *inputDepthModel(const char *path);
struct Coin *createCoin(const int radius, const char trim);
void freeFloatSurface(struct FloatSurface *input);
void freeCoin(struct Coin *penny);
float** createFloatArray(const int cols, const int rows);
void freeFloatArray(float **array, const int rows);
char** createBooleanArray(const int cols, const int rows);
void freeBooleanArray(char **array, const int rows);

// Rolling Coin surface smoothing (safe for navigation): (rolling_coin_smoothing.c)
void coinRollSurface(struct FloatSurface *src, struct Coin *penny);
float getShoalestDepthOnCoin(struct FloatSurface *src, struct Coin *penny, const int row_index, const int col_index);
void getCoinIndexRange(const int rows, const int cols, const int current_row, const int current_col, const int coin_radius, int *index_ranges);
void maxFilterSurface(struct FloatSurface *src);

// Laplacian surface smoothing (safe for navigation): (laplacian_smoothing.c)
void smoothLaplacian(const int iterations, struct FloatSurface *src);
char isNodata(struct FloatSurface *src, int rowindex, int colindex);
float getInterpolatedDepth(struct FloatSurface *src, int row, int col);
float getSafeSmoothDepth(struct FloatSurface *src, int row, int col);

// File output functions: (fileoutput.c)
void parsePath(char *inputfp, char *addon, char *ret);
float *convertFloatArray(struct FloatSurface *input);
void writeSurfaceToFile(struct FloatSurface *input, char *outputpath);

// Printers for help etc:
void printHelp(void);
void printFloatSurfaceInfo(struct FloatSurface *input);
void printCoin(struct Coin *penny);
