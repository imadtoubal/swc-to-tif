# SWC to Tiff stack converter in C++
Convert SWC file format to a 3D volume in multi-page tiff format.

## Usage
Build uins CMake

1. Create and cd into a build directory
    ```
    mkdir build
    cd build
    ```
1. Condifure Cmake
    ```
    cmake ..
    ```
1. Build
    ```
    make
    ```

Run the program
```
./SWC2Tiff <input image> <input swc> <output tiff>
```

|argument|description|
|--------|-----------|
|`input image`| Path to the input image in tiff format. This is only used to get the shape of the volume. |
|`input swc`| Path to the input swc file desired to be converted into a binary volume. |
|`output tiff`| Path to the output binary volume (must end in `.tiff` or `.tif` to support multipage.) |

