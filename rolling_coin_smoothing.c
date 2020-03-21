#include "bathymetrictools.h"

/*
*   This file contains:
*   - Navigationally safe Rolling Coin surface manipulation (2.5D) functions
*   - Coin is defined by structured datatype "Coin"
*/


/*
*   Surface manipulation (~smoothing) function
*   - Iterates over cells
*   - Modifies the surface
*   - Memory management and no data handling
*/
void coinRollSurface(struct FloatSurface *src, struct Coin *penny) {
    printf("Rolling Coin..");
    fflush(stdout);
    const int radius = penny->radius;           // Valid indexes of coin are normally [-radius, radius]
    int *limits = calloc(4, sizeof(int));       // An array to hold valid coin index ranges for special cases
    const float nodata = src->nodata;
    const float placeholder = -999999.0;
    float shoalest;

    // Create new temporary float** (2D) array:
    float **temp = createFloatArray(src->cols, src->rows);

    // Initialize all cells to an elevation of 10 000 (meters):
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {
            temp[row][col] = 10000.0;
        }
    }

    // Iterate over depth model cells and smooth surface:
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {

            // Get coin index limits:
            getCoinIndexRange(src->rows, src->cols, row, col, penny->radius, limits);

            shoalest = placeholder;                                     // Reset shoalest value
            shoalest = getShoalestDepthOnCoin(src, penny, row, col);    // Get actual shoalest value

            if (fabs(shoalest - nodata) > EPSILON) {                                        // Depth values found (!= nodata)
                for (int row_coin = limits[0]; row_coin <= limits[2]; row_coin++) {         // Use valid Coin Row indexes (coin may be partial)
                    for (int col_coin = limits[1]; col_coin <= limits[3]; col_coin++) {     // Use valid Coin Col indexes (coin may be partial)
                        if (penny->array[row_coin + radius][col_coin + radius] == TRUE) {   // On the coin
                            if (temp[row + row_coin][col + col_coin] > shoalest) {          // If current depth of cell is shoaler than shoalest
                                temp[row + row_coin][col + col_coin] = shoalest;            // "Press" depth to coin area
                            }
                        }
                    }
                }
            }
        }
    }

    // Restore original nodata (safety first):
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {
            if ((fabs(src->array[row][col] - nodata) < EPSILON)) {  // (value == nodata)
                temp[row][col] = nodata;
            }
        }
    }

    // Free memory allocated for temp array:
    float **destruct = src->array;          // Store original data array pointer
    src->array = temp;                      // Replace original data array with smoothed data array
    freeFloatArray(destruct, src->rows);    // Free original data array
    free(limits);                           // Free index range list memory
    printf("Done\n");
    fflush(stdout);
}


/*
*   Checks coin area, finds shoalest depth (maximum elevation)
*   - Edge effects: uses partial coin when necessary
*   - Returns shoalest depth on neighborhood (coin area) or 'No data'
*/
float getShoalestDepthOnCoin(struct FloatSurface *src, struct Coin *penny, const int row_index, const int col_index) {
    const float placeholder = -999999.0;
    float ret = placeholder;
    int *limits;

    // Get coin index range:
    limits = calloc(4, sizeof(int));
    getCoinIndexRange(src->rows, src->cols, row_index, col_index, penny->radius, limits);

    // Check coin area using coin index ranges based on current cell:
    for (int row_coin = limits[0]; row_coin <= limits[2]; row_coin++) {
        for (int col_coin = limits[1]; col_coin <= limits[3]; col_coin++) {

            if (penny->array[penny->radius + row_coin][penny->radius + col_coin] == TRUE) {     // If cell is on coin --> check
                if (src->array[row_index + row_coin][col_index + col_coin] > ret) {
                    if (fabs(src->array[row_index + row_coin][col_index + col_coin] - src->nodata) > EPSILON) {  // != NO DATA
                        ret = src->array[row_index + row_coin][col_index + col_coin];
                    }
                }
            }
        }
    }

    free(limits);   // Free index range list memory
    
    if (fabs(ret - src->nodata) < EPSILON) {
        ret = src->nodata;
    }   

    return ret;
}


/*
*   Calculates the allowed range of coin cell indexes
*       - Edge effects are handled by using partial coin
*       - Partial coin is used near data edges and in data corners
*
*   Allowed index ranges are saved to a list passed as a parameter.
*   List contents are as follows [list index]:
*       [0]: Min coin row index
*       [1]: Min coin column index
*       [2]: Max coin row index
*       [3]: Max coin column index
*/
void getCoinIndexRange(const int rows, const int cols, const int current_row, const int current_col, const int coin_radius, int *index_ranges) {
        if (current_row < coin_radius) {
            index_ranges[0] = -current_row;
        }   else {
            index_ranges[0] = -coin_radius;
        }
        
        if (current_col < coin_radius) {
            index_ranges[1] = -current_col;
        }   else {
            index_ranges[1] = -coin_radius;
        }

        if ((rows - 1 - current_row) < coin_radius) {
            index_ranges[2] = rows - 1 - current_row;
        }   else {
            index_ranges[2] = coin_radius;
        }

        if ((cols - 1 - current_col) < coin_radius) {
            index_ranges[3] = cols - 1 - current_col;
        }   else {
            index_ranges[3] = coin_radius;
        }
}
