# Callgrind Logs
This directory holds the callgrind logs that I ran to figure out any performance
bottlenecks in the volume mapper. Multiple logs were generated with different
experiments but the logs here are the most significant.

The major performance lag was in libvtkFreeType because of the
corner annotations in the code. Through the series of experiments I could change
the performance from 10 fps to 25 fps simply by removing the corner annotation.

Here are the log files and corresponding experiment descriptions:

### callgrind.out.19641.Multiple_Rotations_FreeType
This was the base test where 36 10° rolls of the volume were performed. It
includes the first frame as well as the whole main function.

### callgrind.out.23153.Volume_FreeType
This test involved profiling only one step (not the first frame) of the
rotation. As the name suggests, it includes the corner annotation and freetype
calls.
  - libvtkFreeType takes up 77.5% of the total time
  - libvtkRenderingVolumeOpenGL2 takes 0.02% of the time

### callgrind.out.3233.GPURender_Just_Volume
This test involved profiling just one step without the corner annotation i.e.
just the `vtkGPUVolumeRayCastMapper::Render()` call.
  - Most time spent in a single call under `DoGPURender` was for
    `vtkOpenGLGPUVolumeRayCastMapper::vtkInternal::ActivateTransferFunction()`
  - Other major time takers were uniform setters in shader program

### callgrind.out.30130.Multiple_Rotations_JustVolume
This test was to average out the numbers from the prior single run. Just the
volume was rendered and rotated 36 times. Results are consistent with just a
single frame from the previous step.
