#include "bathymetrictools.h"

/*
*   This file contains:
*   - Navigationally safe Laplace interpolation implementation to smooth surfaces iteratively
*   - Neighborhood is only the immediate neighboring cells, nodata is handled as a missing neighbor
*   - Weights are calculated from spatial resolutions in X- and Y-directions
*
*   For example: If spatial resolution X = Y, the used neighborhood (convolution kernel) will be like:
*
*           |0   1   0|
*   1/4  *  |1   0   1|
*           |0   1   0|
*/


/*
*   Controls the iterative smoothing process.
*   - Iterates over surface cells
*   - Memory management
*/
void smoothLaplacian(const int iterations, struct FloatSurface *src) {
    const double nodata = src->nodata;

    // Build extra array to hold smoothed surface (type float**):
    float **smooth_array = createFloatArray(src->cols, src->rows);
    float **holder = NULL;  // Pointer placeholder

    // Iterate and smooth surface N times:
    for (int i = 0; i < iterations; i++) {
        for (int row = 0; row < src->rows; row++) {
            for (int col = 0; col < src->cols; col++) {
                if (fabs(src->array[row][col] - nodata) > EPSILON) {
                    smooth_array[row][col] = getSafeSmoothDepth(src, row, col);
                }   else {
                    smooth_array[row][col] = nodata;
                }
            }
        }

        // Swap surface data array:
        holder = src->array;        // Store pointer temporarily
        src->array = smooth_array;  // Change surface struct data array
        smooth_array = holder;      // Use the same temporary array again
    }

    // Free memory of the temporary array:
    freeFloatArray(smooth_array, src->rows);
}


/*
*   Helper function to check if given cell holds a No Data value.
*   - Returns 1 if cell value == No Data
*   - Returns 0 if cell value != No Data
*/
char isNodata(struct FloatSurface *src, int rowindex, int colindex) {
    if (fabs(src->array[rowindex][colindex] - src->nodata) > EPSILON) {  // != NO DATA
        return 0;
    }  else {
        return 1;
    }
}


/*
*   Interpolates a value based on immediate neighborhood.
*   - A minimum of 2 valid (NoData handled as not valid)
*     neighbor cells is needed to interpolate a value
*   - Handles all cells, note behaviour in corners and borders
*/
float getInterpolatedDepth(struct FloatSurface *src, int row, int col) {
    const double xres = fabs(src->geotransform[1]);  // W-E grid cell spatial resolution
    const double yres = fabs(src->geotransform[5]);  // N-S grid cell spatial resolution (negative value)

    // Get kernel weights (Wi = dVi / di)
    const double xWeight = yres / xres;  // --> X-direction: Yres / Xres
    const double yWeight = xres / yres;  // --> Y-direction: Xres / Yres

    double weightSum = 0.0;
    double list[4] = {0.0, 0.0, 0.0, 0.0};
    double sum = 0;
    int count = 0;

    // Top left corner:
    if (row == 0 && col == 0) {
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Top right corner:
    }   else if (row == 0 && col == src->cols - 1) {
        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Lower left corner:
    }   else if (row == src->rows - 1 && col == 0) {
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Lower right corner:
    }   else if (row == src->rows - 1 && col == src->cols - 1) {

        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Left border:
    }   else if (col == 0) {
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Right border:
    }   else if (col == src->cols - 1) {
        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }

        // Top row:
    }   else if (row == 0) {
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }

        // Bottom row:
    }   else if (row == src->rows - 1) {
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }

        // All other cells (in the middle):
    }   else {
        if (isNodata(src, row - 1, col) == 0) {
            list[count] = src->array[row - 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row + 1, col) == 0) {
            list[count] = src->array[row + 1][col] * yWeight;
            weightSum += yWeight;
            count ++;
        }
        if (isNodata(src, row, col - 1) == 0) {
            list[count] = src->array[row][col - 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
        if (isNodata(src, row, col + 1) == 0) {
            list[count] = src->array[row][col + 1] * xWeight;
            weightSum += xWeight;
            count ++;
        }
    }


    // If less than 2 valid (!= No Data) neighbors, do not interpolate but return cell value itself:
    if (count < 2) {
        return src->array[row][col];

    }   else {  // Return weighted mean of neighbours

        // Calculate sum of cell values:
        for (int i = 0; i < count; i++) {
            sum += list[i];
        }

        return (float)(sum / weightSum);
    }
}


/*
*   Function to guarantee the safety of the process.
*   - Gets an interpolated value based on neighborhood and
*     compares it to the original cell value
*   - Returns the one that is shoaler
*/
float getSafeSmoothDepth(struct FloatSurface *src, int row, int col) {
    const float z = src->array[row][col];                           // Original value
    const float estimate = getInterpolatedDepth(src, row, col);     // Interpolated value
    
    // Return safer value:
    if (fabs(estimate) < fabs(z)) {
        return estimate;
    }   else {
        return z;
    }
}
