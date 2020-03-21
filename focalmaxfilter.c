#include "bathymetrictools.h"

/*
*   This file contains:
*   - Shoal buffering function
*   - Essentially a 3 x 3 cell focal maximum filter
*/


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
