#include "bathymetrictools.h"

/*
*   This file contains:
*   - Printer functions for structured data types sto help with development
*   - Help
*/

/*
*   Prints help
*/
void printHelp(void) {
    printf("Bathymetric surface tools - help:\n");
    printf("\n 1. To launch user interface use -ui flag:\n\n\tsurfacetools -ui\n");
    printf("\n 2. CLI (use for scripting):\n\n\tsurfacetools [full inputfilepath] [full outputfilepath] -methodflag P -methodflag P -trimflag(Rolling Coin only) \n");
    printf("\n\tMethods:\n\t  -buffer = Buffer shoals (3x3 cell focal max filter)\n\t\t* No parameters\n\t\t* Use example: surfacetools [inputfile] [outputfile] -buffer");
    printf("\n\t  -laplacian = Laplacian smoothing\n\t\t* Parameters: [N] = number of iterations (integer)\n\t\t* Use example: surfacetools [inputfile] [outputfile] -laplacian 25");
    printf("\n\t  -rollcoin = Rolling Coin smoothing\n\t\t* Parameters: [R] = coin radius in cells (integer), [trim/notrim] = trim flag (coin edge trimming)");
    printf("\n\t\t* Use example: surfacetools [inputfile] [outputfile] -rollcoin 15 trim");
    printf("\n\n\tExamples:\n");
    printf("\t\tBuffer shoals: surfacetools inputfile.tiff outputfile.tiff -buffer\n");
    printf("\t\tLaplacian smoothing, 10 iterations: surfacetools inputfile.tiff outputfile.tiff -laplacian 10\n");
    printf("\t\tRolling Coin, radius 15, trimmed coin edges: surfacetools inputfile.tiff outputfile.tiff -rollcoin 15 trim\n");
    printf("\t\tChaining process steps (Shoal Buffering >> Rolling Coin >> Laplacian Smoothing):\n\t\t  surfacetools inputfile.tiff outputfile.tiff -buffer -rollcoin 13 notrim -laplacian 10\n\n");
}


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
