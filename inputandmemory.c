#include "bathymetrictools.h"

/*
*   This file contains:
*   - File input functions
*   - Stuctured datatype builders
*   - Memory management functions: allocates and frees
*/


/*
*   Builds a structured datatype (type "FloatSurface") variable from input depth model. 
*   - Allocates memory and populates struct
*   - Returns pointer to FloatSurface
*   - PATH parameter:
*       - If NULL, asks user to define input path
*       - If path is passed as a parameter, skips user input stage (CLI use)
*/
struct FloatSurface *inputDepthModel(const char *path) {
    int len = 1000;                                                     // Space for 1000 characters
    char filepath[len];

    // Stage 1: Get file path if NULL was passed as a parameter    
    if (path == NULL) {
        printf("\nEnter full filepath to bathymetric surface (e.g. /users/john/desktop/test.bag):\n");
        fgets(filepath, len, stdin);
        len = strlen(filepath);
        filepath[len-1] = '\0';
    }   else {                                                          // Path passed as a parameter
        strcpy(filepath, path);
    }

    // Stage 2: Open dataset
    GDALDatasetH dataset = NULL;
    GDALRasterBandH band;
    GDALAllRegister();                                                  // Register all GDAL drivers
    int success;

    if (access(filepath, R_OK|W_OK) != -1) {                            // Check that file exists (read & write permissions ok)
        dataset = GDALOpen(filepath, GA_ReadOnly);                      // Try to open dataset
    } else {
        printf("File read error. Recheck file path.\nExiting.\n");
        exit(EXIT_FAILURE);
    }
 
    if (dataset == NULL) {
        printf("File read error. Recheck file path.\nExiting.\n");
        exit(EXIT_FAILURE);
    }   else{
        printf("File read successful. Building surface..");
    }

    band = GDALGetRasterBand(dataset, 1);                               // Get raster band

    // Allocate and populate struct:
    struct FloatSurface *ret = calloc(1, sizeof(struct FloatSurface));  // Allocate memory for FloatSurface
    len = strlen(filepath);                                             // Get filepath length
    ret->inputfp = calloc(len + 1, 1);                                  // Allocate memory for null character too (+1)
    ret->inputfp = strcpy(ret->inputfp, filepath);                      // Copy filepath string to struct

    const char *src_projection =  GDALGetProjectionRef(dataset);        // Get projection information
    len = strlen(src_projection);
    ret->projection = calloc(len + 1, 1);                               // +1 for null character
    ret->projection = strcpy(ret->projection, (char*)src_projection);   // Set projection information

    ret->geotransform = calloc(6, sizeof(double));                      // Allocate memory for 6 doubles
    GDALGetGeoTransform(dataset, ret->geotransform);                    // Set geotransform parameters

    ret->nodata = GDALGetRasterNoDataValue(band, &success);             // Set nodata value
    ret->rows = GDALGetRasterBandYSize(band);                           // Set row count
    ret->cols = GDALGetRasterBandXSize(band);                           // Set column count

    // Allocate memory & get depth model data as an array (float**):
    float **array = calloc(ret->rows, sizeof(float *));                 // Allocate memory for rows
    for (int row = 0; row < ret->rows; row++ ) {
        array[row] = calloc(ret->cols, sizeof(float));                  // Allocate memory for columns
    }

    // Allocate memory for temporary 1D array to read all data at once
    float *temp = CPLMalloc(sizeof(float) * ret->rows * ret->cols);

    // Read data:
    CPLErr err = GDALRasterIO(band,
        GF_Read,
        0,              // x offset
        0,              // y offset
        ret->cols,      // x size
        ret->rows,      // y size
        temp,           // data array
        ret->cols,      // x buffer size
        ret->rows,      // y buffer size
        GDT_Float32,    // datatype
        0,              // pixel space
        0);             // line space

    if (err != CPLE_None) {
        printf("An error occured when reading the input data file: %s\n", CPLGetLastErrorMsg());
    }

    // Convert data to a 2D array:
    int i = 0;   // 1D array cell index
    for (int row = 0; row < ret->rows; row++) {
         for (int col = 0; col < ret->cols; col++) {
            array[row][col] = temp[i];
            i++;
        }
    }

    // Free temp array memory:
    CPLFree(temp);

    ret->array = array;     // Store data array pointer to Struct

    GDALClose(dataset);     // Data is now stored in struct, file can be closed
    printf("Done\n");
    return ret;             // Return struct pointer
}


/*
*   Builds Coin
*   - Allocates memory, returns a pointer to Coin
*   - Input parameters: 
*       - Coin radius (diameter = 2r+1)
*       - Trim flag: 
*           - 0: No trim
*           - 1: Trim outer edges (diameter -= 2)
*/
struct Coin *createCoin(const int radius, const char trim) {
    struct Coin *ret = calloc(1, sizeof(struct Coin));
    char **trimmed;
    char **untrimmed;
    const int untrimmedDiameter = 2 * radius + 1;

    // Set coin radius attribute:
    if (trim == TRUE) {
        ret->radius = radius - 1;
    }   else {
        ret->radius = radius;
    }

    // Set coin diameter attribute:
    ret->diameter = 2 * ret->radius + 1;

    // Array building:
    if (trim == TRUE) {     // Build both arrays if trim is TRUE
        trimmed = createBooleanArray(ret->diameter, ret->diameter);
        untrimmed = createBooleanArray(untrimmedDiameter, untrimmedDiameter);
    }   else {              // Build only untrimmed array if trim is FALSE
        untrimmed = createBooleanArray(untrimmedDiameter, untrimmedDiameter);
    }
    
    // Create coin (untrimmed array):
    for (int i = 0; i < untrimmedDiameter; i++) {
        for (int j = 0; j < untrimmedDiameter; j++) {
            int distY = i - radius;
            int distX = j - radius;

            if ((distX * distX) + (distY * distY) <= (radius * radius)) {  // On (round) coin --> TRUE
                untrimmed[i][j] = TRUE;
            }   else {
                untrimmed[i][j] = FALSE;  // Not on coin
            }
        }
    }

    // Trim coin if selected:
    if (trim == TRUE) {
        for (int i = 0; i < ret->diameter; i++) {
            for (int j = 0; j < ret->diameter; j++) {
                trimmed[i][j] = untrimmed[i + 1][j + 1];
            }
        }

        freeBooleanArray(untrimmed, ret->diameter); // Free memory of extra array
        ret->array = trimmed;
    
    }   else {                                      // No extra arrays to free
        ret->array = untrimmed;
    }

    return ret;
}


/*
*   Frees all allocated memory of parameter FloatSurface
*   All FloatSurfaces must be freed in order to avoid memory leaks
*/
void freeFloatSurface(struct FloatSurface *input) {
    float **array = input->array;
    int rows = input->rows;
    free(input->inputfp);           // Free input filepath
    free(input->projection);        // Free CRS WKT string
    free(input->geotransform);      // Free geotrans parameters
    freeFloatArray(array, rows);    // Free data array
    free(input);                    // Free struct
}


/*
*   Frees all allocated memory of parameter Coin
*   All Coins must be freed in order to avoid any memory leaks
*/
void freeCoin(struct Coin *penny) {
    freeBooleanArray(penny->array, penny->diameter);    // Free array
    free(penny);                                        // Free Struct
}


/*
*   - Allocates memory for (2D) float** array of given size
*   - Returns a pointer to array
*/
float** createFloatArray(const int cols, const int rows) {
    float **ret;                                    // Array pointer to be returned
    ret = calloc(rows, sizeof(float*));             // Allocate memory for rows

    for (int row = 0; row < rows; row++) {
        ret[row] = calloc(cols, sizeof(float));     // Allocate memory for columns
    }

    return ret;
}


/*
*   Frees allocated memory of 2D float (float**) array
*/
void freeFloatArray(float **array, const int rows) {
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }

    free(array);
}


/*
*   - Allocates memory for (2D) char** array of given size
*   - Returns a pointer to array
*/
char** createBooleanArray(const int cols, const int rows) {
    char **ret;                             // Array pointer to be returned
    ret = calloc(rows, sizeof(char*));      // Allocate memory for rows

    for (int row = 0; row < rows; row++) {
        ret[row] = calloc(cols, 1);         // Allocate memory for columns
    }

    return ret;
}


/*
*   Frees allocated memory of 2D char (char**) array
*/
void freeBooleanArray(char **array, const int rows) {
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }

    free(array);
}
