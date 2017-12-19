# vtkColorCodedDepthVolume

vtkColorCodedDepthVolume is a sample application that demonstrates animating a
sequence of NRRD volumes with custom blend modes in VTK.

# Features

- Switch between different volume rendering modes
  - Composite blending (stock volume rendering)
  - Average intensity projection
  - Maximum intensity projection
  - Color coding the depth of first non-transparent voxel along the ray. (Think
    of this as a colored depth map).
  - Color coding based on the depth of individual voxels (yellow-brown-blue
    color function).
- Demonstrates use of custom shaders in VTK volume rendering
- Read and animate a sequence of NRRD volumes in VTK
- Licensed under Apache version 2.0

# Compile

Use CMake to configure the build system for the platform of choice. Build the
configured system using the generator selected in CMake.
For more details, take a look at the [Running
CMake](https://cmake.org/runningcmake/) tutorial. 

# Usage

```shell_session
$ ./vtkColorCodedDepthVolume <Nrrd files directory>
```

The argument should point to the directory containing the time sequence of NRRD
volumes as individual files.

**Note** Take a look at [Preparing Data](#preparing-data) to convert a single
NRRD sequence file into multiple files.

Once the above command is run, the application starts reading each file in the
directory as a time series and instantiates a volume pipeline for each dataset.
A default color and scalar opacity transfer function are applied for the
composite blend mode. One can now use the mouse to interact with the rendered
data. Use the custom keymappings below to interact with the whole series:

| Key             |         Mapping                |
|:----------------|:-------------------------------|
|  Space          |  Play / Loop through sequence  |
|   n             |      Next volume               |
|   p             |      Previous volume           |
|   a             |   Average Intensity Projection |
|   m             |   Maximum Intensity Projection |
|   c             |   Composite Blend (default)    |
|   d             |   Color coded depth map (First non-transparent voxel) |
|   o             |   Color coded depth            |

# Preparing Data

For converting a single `*.seq.nrrd` (4D NRRD volume) into individual 3D NRRD
volumes, one can use [teem-unu](http://teem.sourceforge.net/unrrdu/).

```shell_session
$ unu dice -a 0 -i <input 4D file> -o <output basename> -ff %03.nrrd
```
