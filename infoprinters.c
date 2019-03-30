#include "bathymetrictools.h"

/*
*   This file contains:
*   - Printer functions for structured data types
*     to help with development. 
*   - These are not strictly necessary (but used in current main, so included also here)
*/


/*
*   Prints info of depth model surface
*/
void printFloatSurfaceInfo(struct FloatSurface *input) {
    printf("\n\nStruct FloatSurface content:\n");
    printf("Filepath (input): %s\n", input->inputfp);
    printf("\nProjection: %s\n", input->projection);
    printf("\nNodata value: %f\n", input->nodata);
    printf("Rows: %d, Columns: %d\n", input->rows, input->cols);
    printf("Georeferencing information: %f, %f, %f, %f, %f, %f \n", input->geotransform[0], input->geotransform[1], input->geotransform[2], input->geotransform[3], input->geotransform[4], input->geotransform[5]);
    printf("Test from array[%d][%d]: %f\n\n", input->rows / 2, input->cols / 2, input->array[input->rows / 2][input->cols / 2]);
}


/*
*   Prints coin
*/
void printCoin(struct Coin *penny) {
    printf("\nCoin information:\nDiameter: %d (Radius: %d)\n\n", penny->diameter, penny->radius);
    for (int row = 0; row < penny->diameter; row++) {
        for (int col = 0; col < penny->diameter; col++) {
            if (penny->array[row][col] == 1) {
                printf("1");
            }   else {
                printf(" ");
            }
            printf(" ");
        }
        printf("\n");
    }
}
