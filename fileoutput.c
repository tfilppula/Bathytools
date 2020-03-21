#include "bathymetrictools.h"

/*
*   This file contains:
*   - File output related functions
*/


/*
*   Simple output filepath parser
*/
void parsePath(char *inputfp, char *addon, char *ret) {
    int leninput = strlen(inputfp);
    int index = 0;

    while (index < leninput) {
        if (inputfp[index] == '.') {
            ret[index] = '\0';
            break;
        }
        ret[index] = inputfp[index];
        index++;
    }
 
    strcat(ret, addon);
}


/*
*   Converts FloatSurface 2D float** arrays to (1D) float* arrays
*   for file output
*/
float *convertFloatArray(struct FloatSurface *input) {
    int index = 0;

    float *ret = CPLMalloc(sizeof(float) * input->rows * input->cols);

    for (int row = 0; row < input->rows; row++) {
        for (int col = 0; col < input->cols; col++) {
            ret[index] = input->array[row][col];
            index++;
        }
    }

    return ret;
}


/*
*   Writes FloatSurface to a GeoTIFF file
*   - Uses GDAL for I/O
*/
void writeSurfaceToFile(struct FloatSurface *input, const char *outputpath) {
    printf("Exporting file..");
    fflush(stdout);
    GDALAllRegister();
    const char *format = "GTiff";
    GDALDriverH driver = GDALGetDriverByName(format);
    char **papszOptions = NULL;
    char outputfp[1000];

    // Parse output filename if NULL was passed as parameter
    if (outputpath == NULL) {
        parsePath(input->inputfp, "_smoothed_surface.tif", outputfp);
    }   else {
        strcpy(outputfp, outputpath);
    }

    papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "DEFLATE" );
    GDALDatasetH outdataset = GDALCreate(driver, outputfp, input->cols, input->rows, 1, GDT_Float32, papszOptions);
    GDALRasterBandH outband = GDALGetRasterBand(outdataset, 1);
    GDALSetGeoTransform(outdataset, input->geotransform);
    GDALSetProjection(outdataset, input->projection);

    float *datalist = convertFloatArray(input);

    char ret = GDALRasterIO(outband, GF_Write, 0, 0, input->cols, input->rows, datalist, input->cols, input->rows, GDT_Float32, 0, 0);

    if (ret != 0) {
        printf("Export was not successful.\n");
    }
    
    GDALSetRasterNoDataValue(outband, input->nodata);       
    
    free(datalist);
    GDALClose(outdataset);
    printf("Done. Surface exported to file: %s\n\n", outputfp);
    fflush(stdout);
}
