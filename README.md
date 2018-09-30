# vtkColorCodedDepthVolume

vtkColorCodedDepthVolume is a sample application that demonstrates animating a
sequence of NRRD volumes with custom blend modes in VTK.

## Features

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

## Compile

Use CMake to configure the build system for the platform of choice. Build the
configured system using the generator selected in CMake.
For more details, take a look at the [Running
CMake](https://cmake.org/runningcmake/) tutorial.

## Usage

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

### Preparing Data

For converting a single `*.seq.nrrd` (4D NRRD volume) into individual 3D NRRD
volumes, one can use [teem-unu](http://teem.sourceforge.net/unrrdu/).

```shell_session
$ unu dice -a 0 -i <input 4D file> -o <output basename> -ff %03d.nrrd
```

## Description

The source for vtkColorCodedDepthVolume consists of:

- [vtkColorCodedDepthVolume.cxx](./vtkColorCodedDepthVolume.cxx):
  This file provides the `main()` function for the application that sets up the
  rendering pipeline as well as defines
  relevant callbacks for the different key-mappings as described in
  [Usage](#usage).
- [vtkNrrdSequenceReader.h(cxx)](./vtkNrrdSequenceReader.h): This is a VTK
  reader that reads in multiple NRRD files from a single directory as individual
  time steps. The output of the reader is a `vtkImageData` object of the first
  timestep by default. The reader provides API for specifying a particular
  timestep as well as convenience API like `Next()` and `Previous()`.
- [ColorCodedDepthFragmentShader.glsl](./ColorCodedDepthFragmentShader.glsl):
  This is a custom fragment shader that is used for color coding the depth of
  first non-transparent voxel along the ray. To simplify consumption of the
  shader code in the C++ application, it is converted to a C string using
  [vtkEncodeString.cmake](https://gitlab.kitware.com/vtk/vtk/blob/590b3413ba71bfe0a746d276360d055001ec8eac/CMake/vtkEncodeString.cmake).

### Custom shaders

This application also serves as an example for using custom GLSL shading code
in VTK's volume rendering framework. There are two types of customizations
available to the user - partial replacements or full shader replacements.

The application uses
[`vtkOpenGLGPUVolumeRayCastMapper::AddShaderReplacement()`](https://www.vtk.org/doc/nightly/html/classvtkOpenGLGPUVolumeRayCastMapper.html#a5f701007fb5301dfd2b047739ab28edc)
to modify final color computation in the fragment shader. The final image is a
composite representing depth of individual voxels in a yellow - brown - blue
color transfer function.

The application uses
[`vtkOpenGLGPUVolumeRayCastMapper::SetFragmentShaderCode()`](https://www.vtk.org/doc/nightly/html/classvtkOpenGLGPUVolumeRayCastMapper.html#a95123c088bb25fbc8231702bedf57aea) to replace the
whole shader code with the code provided by
[ColorCodedDepthFragmentShader.glsl](./ColorCodedDepthFragmentShader.glsl).
This code terminates the ray at the first non-transparent voxel and registers
its depth. The final color of the rendered fragment is decided based on the
depth value registered for the fragment. In this case, the color transfer
function is defined over the Z-range i.e. (0, depth) range of the volume. We set
[`vtkGPUVolumeRayCastMapper::SetColorRangeType`](https://www.vtk.org/doc/nightly/html/classvtkGPUVolumeRayCastMapper.html#a784d19bb8eeb1350269dbc99e7be995f)
to `NATIVE` to avoid re-scaling the transfer function over the scalar range in
the mapper.
