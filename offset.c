#include "bathymetrictools.h"

/*
*   This file contains:
*   - Surface vertical offset function
*   - Offset is defined as a local addition of offset value to cell value
*   - Skips all NoData cells, affects only cells with data
*/


/*
*   Offset
*/
void offset(struct FloatSurface *src, const float offset) {
    printf("Offsetting surface..");
    fflush(stdout);
    float nodata = src->nodata;
    float depth;

    // Iterate over cells and offset surface:
    for (int row = 0; row < src->rows; row++) {
        for (int col = 0; col < src->cols; col++) {
            // Only update cells that are not NoData:
            if (fabs(src->array[row][col] - nodata) < EPSILON) {
                continue;
            } 
            depth = src->array[row][col] + offset;
            src->array[row][col] = depth;
        }
    }

    printf("Done\n");
    fflush(stdout);
}
