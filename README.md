# Bathymetric surface tools
This repository holds bathymetric surface manipulation tools that I developed originally for my Master's thesis (thesis available online, in Finnish only: https://helda.helsinki.fi/handle/10138/273488)

This source code includes efficient implementations (written in pure C) of Rolling Coin surface smoothing algorithm (for more information please refer to my thesis) and navigationally safe iterative Laplacian interpolation.

Dependencies:
  - GDAL library (for file I/O)

Disclaimer:
  - Source is provided as is, with absolutely no quarantees of any kind
  
----

Compile using make and makefile (provided) or using for example gcc or clang (link gdal when compiling):

```
gcc -g -O3 -march=native -Wall -Wextra -Wfloat-equal -Werror -std=gnu11 -o bathytools *.c -lgdal
```
----
These tools includes a Command Line Interface and also a simple text-based UI. Available methods are:
* Rolling Coin surface smoothing (see my thesis for reference)
* Laplacian smoothing (navigationally safe iterative Laplacian interpolation)
* Shoal expansion (3x3 cell focal maximum filter)
* Surface offset (local addition to cell value)
More information can also be found in the corresponding source files.

Methods can be chained together for example as follows:
```
surfacetools inputfile.tiff outputfile.tiff -buffer -rollcoin 13 notrim -laplacian 10 -offset 0.35
```
The above example does the following:
1. Apply Shoal buffering (3x3 cell focal maximum filter)
2. Apply Rolling Coin smoothing to buffered surface (Coin radius = 12 cells, no trimming of coin edges)
3. Apply Laplacian smoothing (10 iterations)
4. Lastly, apply an offset of +0.35 m for every grid cell
