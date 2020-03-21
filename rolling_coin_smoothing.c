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


/*
*   Buffers shoals by one cell to all directions.
*   - Used to expand shoals to ensure the safety of depth contours
*   - Uses a 3 x 3 cell focal maximum filter:
*
*                   + + +
*                   + X +
*                   + + +
*
*   - Cell X gets the shoalest value of neighborhood
*   - Neighborhood is marked with "+"
*       - If X is shoalest, cell value doesn't change
*/
void maxFilterSurface(struct FloatSurface *src) {
    printf("Buffering shoals..");
    fflush(stdout);
    float nodata = src->nodata;
    float max_elev = -15000.0;  // Placeholder for shoalest depth
    const float placeholder = max_elev;
    int lenlist = 8;
    float neighborhood[lenlist];

    // Create new float** (2D) array:
    float **temp = createFloatArray(src->cols, src->rows);

    // Make a temporary copy of the original data array:
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {
            temp[row][col] = src->array[row][col];
        }
    }

    // Iterate over cells and filter surface:
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {

            // Reset max_elev to placeholder value:
            max_elev = placeholder;

            // Initialize / reset neighborhood array:
            lenlist = 8;
            for (int i = 0; i < lenlist; i++) {
                neighborhood[i] = placeholder;
            }

            if (row == 0) {                                 // Top row
                if (col == 0) {                                 // Top-left
                    neighborhood[0] = temp[row][col + 1];
                    neighborhood[1] = temp[row + 1][col];
                    neighborhood[2] = temp[row + 1][col + 1];
                    lenlist = 3;

                }   else if (col == src->cols - 1) {            // Top-right
                    neighborhood[0] = temp[row][col - 1];
                    neighborhood[1] = temp[row + 1][col];
                    neighborhood[2] = temp[row + 1][col -1];
                    lenlist = 3;

                }   else {                                      // Between corners
                    neighborhood[0] = temp[row][col - 1];
                    neighborhood[1] = temp[row][col + 1];
                    neighborhood[2] = temp[row + 1][col];
                    neighborhood[3] = temp[row + 1][col - 1];
                    neighborhood[4] = temp[row + 1][col + 1];
                    lenlist = 5;
                }

            }   else if (row == src->rows - 1) {            // Bottom row
                if (col == 0) {                                 // Bottom-left
                    neighborhood[0] = temp[row][col + 1];
                    neighborhood[1] = temp[row - 1][col];
                    neighborhood[2] = temp[row - 1][col + 1];
                    lenlist = 3;

                }   else if (col == src->cols - 1) {            // Bottom-right
                    neighborhood[0] = temp[row][col - 1];
                    neighborhood[1] = temp[row - 1][col];
                    neighborhood[2] = temp[row - 1][col - 1];
                    lenlist = 3;
                
                }   else {                                      // Between corners
                    neighborhood[0] = temp[row][col - 1];
                    neighborhood[1] = temp[row][col + 1];
                    neighborhood[2] = temp[row - 1][col];
                    neighborhood[3] = temp[row - 1][col - 1];
                    neighborhood[4] = temp[row - 1][col + 1];
                    lenlist = 5;
                }

            }   else {                                      // Rows in between top and bottom
                if (col == 0) {                                 // Left edge
                    neighborhood[0] = temp[row][col + 1];
                    neighborhood[1] = temp[row + 1][col];
                    neighborhood[2] = temp[row - 1][col];
                    neighborhood[3] = temp[row - 1][col + 1];
                    neighborhood[4] = temp[row + 1][col + 1];
                    lenlist = 5;

                }   else if (col == src->cols - 1) {            // Right edge
                    neighborhood[0] = temp[row][col - 1];
                    neighborhood[1] = temp[row + 1][col];
                    neighborhood[2] = temp[row - 1][col];
                    neighborhood[3] = temp[row - 1][col - 1];
                    neighborhood[4] = temp[row + 1][col - 1];
                    lenlist = 5;

                }   else {                                      // Between edges
                    neighborhood[0] = temp[row - 1][col - 1];
                    neighborhood[1] = temp[row - 1][col];
                    neighborhood[2] = temp[row - 1][col + 1];
                    neighborhood[3] = temp[row][col - 1];
                    neighborhood[4] = temp[row][col + 1];
                    neighborhood[5] = temp[row + 1][col - 1];
                    neighborhood[6] = temp[row + 1][col];
                    neighborhood[7] = temp[row + 1][col + 1];
                    lenlist = 8;
                }
            }

            // Get max elevation (shoalest depth):
            for (int i = 0; i < lenlist; i++) {
                if (neighborhood[i] > max_elev) {
                    max_elev = neighborhood[i];
                }
            }

            // Update source array cell values if max_elev is shoaler and max_elev is not nodata:
            if (max_elev > src->array[row][col] && fabs(max_elev - nodata) > EPSILON && fabs(src->array[row][col] - nodata) > EPSILON) {
                src->array[row][col] = max_elev;
            }
        }
    }

    // Free temporary data array
    freeFloatArray(temp, src->rows);
    printf("Done\n");
    fflush(stdout);
}
