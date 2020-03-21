#include "bathymetrictools.h"

/*
*   This file contains:
*   - Command line interface specific functions
*/


/*
*   Command line interface logic.
*   v0.1, first implementation
*   21.3.2020
*/
void cli(int argc, const char *argv[]) {
    char inputflag = 1;         // Inputs assumed to be ok
    char trimflag = 0;          // Coin trim flag
    char rollingcoin = 0;       // Method flag to be used for freeing coin memory
    struct Coin *penny;         // Placeholder for Coin

    // Check input file existence and permissions:
    if (access(argv[1], R_OK|W_OK) != 0) {
        printf("File read error. Recheck file path.\nExiting.\n");
        exit(EXIT_FAILURE);
    }

    // Check input process commands and parameters:
    printf("Process steps:\n");
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-buffer") == 0) {
            printf("  -Buffer shoals\n");
            continue;
        }   else if (strcmp(argv[i], "-laplacian") == 0 && argc > i+1) {
            if (atoi(argv[i+1]) > 0) {
                printf("  -Laplacian smoothing, %d iterations\n", atoi(argv[i+1]));
                i++;
            }
        }   else if (strcmp(argv[i], "-rollcoin") == 0 && argc > i+2) {
            if (atoi(argv[i+1]) > 0) {
                if (strcmp(argv[i+2], "trim") == 0 || strcmp(argv[i+2], "notrim") == 0) {
                    printf("  -Rolling Coin: r=%d cells, %s\n", atoi(argv[i+1]), argv[i+2]);
                    i+=2;
                }
            }
        }
        else {
            inputflag = 0;
        }
    }

    // Terminate process if invalid parameters are given:
    if (inputflag != 1) {
        printf("Faulty parameters detected. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    // Start processing surface:
    // 1. Open surface
    struct FloatSurface *surf = inputDepthModel(argv[1]);
    
    // 2. Perform process steps
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-buffer") == 0) {
            // Apply 3x3 cell focal maximun filter:
            maxFilterSurface(surf);
        }   else if (strcmp(argv[i], "-laplacian") == 0 && argc > i+1) {
            if (atoi(argv[i+1]) > 0) {
                // Apply Laplacian smoothing:
                smoothLaplacian(atoi(argv[i+1]), surf);
                i++;
            }
        }   else if (strcmp(argv[i], "-rollcoin") == 0 && argc > i+2) {
            if (atoi(argv[i+1]) > 0) {
                if (strcmp(argv[i+2], "trim") == 0 || strcmp(argv[i+2], "notrim") == 0) {
                    // Apply Rolling Coin smoothings
                    // Set trim flag:
                    if (strcmp(argv[i+2], "trim") == 0) {
                        trimflag = 1;
                    }   else if (strcmp(argv[i+2], "notrim") == 0) {
                        trimflag = 0;
                    }
                    // Set flag to make sure allocated coin is freed
                    rollingcoin = 1;
                    // Create Coin:
                    penny = createCoin(atoi(argv[i+1]), trimflag);
                    // Apply Rolling Coin smoothing:
                    coinRollSurface(surf, penny);
                    i+=2;
                }
            }
        }
    }

    // 3. Write surface to file, path from input parameters:
    writeSurfaceToFile(surf, argv[2]);

    // 4. Free allocated memory of surface & coin objects:
    freeFloatSurface(surf);
    if (rollingcoin == 1) {
        freeCoin(penny);
    }
}
