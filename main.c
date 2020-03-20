#include "bathymetrictools.h"

/*
*   This file contains:
*   - Main function
*   - Input parameter functions
*   - Process functions to export contours and
*     smoothed surfaces
*/


/*
*   Simple main function:
*/
int main(int argc, const char *argv[]) {

    if (argc == 2 && strcmp(argv[1], "-ui") == 0) {
        clearScreen();

        while (1) {
            // Print menu:
            printf("Bathymetric surface tools - select option:\n\
                \n    1. Rolling Coin surface smoothing (Navigationally safe 2.5D surface smoothing) \
                \n    2. Laplacian surface smoothing    (Navigationally safe 2.5D surface smoothing)\
                \n    3. Test coin radius and trimming \
                \n    4. Clear screen \
                \n    5. Exit \
                \n\n");

            char action = intInput(1, 5, "Choose action: ");

            if (action == 1) {
                rollingCoinSmoothing();

            }   else if (action == 2) {
                laplacianSmoothing();

            }   else if (action == 3) {
                testCoins();

            }   else if (action == 4) {
                clearScreen();

            }   else if (action == 5) {
                clearScreen();
                break;
            }
        }
    }   else if (argc > 3) {
        printf("CLI-toiminnot tänne\n");
    }   else {
        clearScreen();
        printHelp();
    }

    return 0;
}


/*
*   Rolling Coin surface smoothing process.
*   - 2.5D
*   - Safe for navigation
*   - With or without shoal buffering (3x3 max filter)
*/
void rollingCoinSmoothing(void) {
    clearScreen();
    printf("Rolling Coin surface smoothing (2.5D):\n");

    // Import data and build a "FloatSurface" (NULL:ask for input path):
    struct FloatSurface *surf = inputDepthModel(NULL);

    // Use shoal buffering?
    char buffering = intInput(0, 1, "Buffer shoals to ensure contour safety? (0: No buffering, 1: Buffer shoals): ");

    // Build a "Coin":
    int coin_r = intInput(1, 50, "Enter coin radius in cells (for example 5): ");
    char trim = intInput(0, 1, "Trim coin edges? (0: No trim, 1: Trim outer edges): ");
    struct Coin *penny = createCoin(coin_r, trim);

    // Buffer shoals if buffering is selected:
    if (buffering == 1) {
        printf("\nBuffering shoals..\n");
        maxFilterSurface(surf);
    }

    // Generalize/smooth surface:
    printf("Smoothing surface..\n");
    fflush(stdout);
    coinRollSurface(surf, penny);

    // Export file (GeoTIFF format):
    printf("Exporting file.. ");
    fflush(stdout);
    // Use default output filename:
    writeSurfaceToFile(surf, NULL);

    // Free allocated memory:
    printf("Freeing memory..");
    freeCoin(penny);
    freeFloatSurface(surf);
    printf("done\n\n");
}


/*
*   Iterative Laplacian interpolation based surface smoothing.
*   Safe for navigation.
*/
void laplacianSmoothing(void) {
    clearScreen();
    printf("Laplacian surface smoothing (2.5D):\n");

    // Import data and build a "FloatSurface" (NULL:ask for input path):
    struct FloatSurface *surf = inputDepthModel(NULL);
    
    // Number of iterations:
    int iterations = intInput(1, 500, "Enter number of smoothing iterations (1 - 500): ");

    // Smooth:
    printf("Smoothing surface.. ");
    fflush(stdout);
    smoothLaplacian(iterations, surf);
    printf("Done\n");
    fflush(stdout);

    // Export file (GeoTIFF format):
    printf("Exporting file.. ");
    fflush(stdout);
    // Use default output filename:
    writeSurfaceToFile(surf, NULL);

    // Free allocated memory:
    printf("Freeing memory..");
    freeFloatSurface(surf);
    printf("done\n\n");
}


/*
*   Function to test coin creation. 
*   - Prints coin on screen
*   - Development aid
*/
void testCoins(void) {
    clearScreen();
    printf("Test coins:\n");
    int coin_r = intInput(1, 50, "Enter coin radius in cells (for example 5): ");
    char trim = intInput(0, 1, "Trim coin edges? (0: No trim, 1: Trim outer edges): ");
    struct Coin *penny = createCoin(coin_r, trim);
    printCoin(penny);
    freeCoin(penny);
    printf("\n");
}


/*
*   Function to clear the screen.
*   - Somewhat portable, somewhat ugly
*/
void clearScreen(void) {
    system("clear||cls"); 
}


/*
*   Function for integer input:
*/
int intInput(const int lower, const int upper, const char *text) {
    char st[40];
    int result = 0;
    int number;
    printf("%s", text);

    while (result != 1 || number < lower || number > upper) {
        fgets(st, sizeof(st), stdin);
        result = sscanf(st, "%d", &number);
        if (result != 1 || (number < lower || number > upper)){
            printf("Invalid input. Enter a number between [%d, %d]: ", lower, upper);
        }
    }

    return number;
}
