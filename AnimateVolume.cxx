/*=========================================================================

  Program:   Visualization Toolkit
  Module:    AnimateVolume.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// VTK includes
#include <vtkActor.h>
#include <vtkAnimationCue.h>
#include <vtkAnimationScene.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

// STL includes
#include <sys/stat.h>
#include <sys/types.h>

// AnimateVolume includes
#include "vtkNrrdSequenceReader.h"
#include <ColorCodedDepthFragmentShader.h>

class ChangeSequenceStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static ChangeSequenceStyle* New();
  vtkTypeMacro(ChangeSequenceStyle, vtkInteractorStyleTrackballCamera);

  virtual void OnKeyPress() override
  {
    // Get the keypress
    vtkRenderWindowInteractor* rwi = this->Interactor;
    std::string key = rwi->GetKeySym();
    // Handle the next volume key
    if (key == "n")
    {
      this->Reader->Next();
    }
    else if (key == "p")
    {
      this->Reader->Previous();
    }
    else if (key == "space")
    {
      this->Reader->SetCurrentIndex(0);
      rwi->GetRenderWindow()->Render();
      for (int i = 0; i < this->Reader->GetNumberOfNrrdFiles() - 1; ++i)
      {
        this->Reader->Next();
        rwi->GetRenderWindow()->Render();
      }
    }
    // Forward the event
    vtkInteractorStyleTrackballCamera::OnKeyPress();
    rwi->GetRenderWindow()->Render();
  }

  vtkNrrdSequenceReader* Reader;
};
vtkStandardNewMacro(ChangeSequenceStyle);

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <Nrrd files directory> "
              << std::endl;
    return EXIT_FAILURE;
  }

  // Basic argument checks
  struct stat info;
  if (stat(argv[1], &info) != 0)
  {
    std::cerr << "ERROR: Cannot access \"" << argv[1] << "\"" << std::endl;
    return EXIT_FAILURE;
  }
  else if ((info.st_mode & S_IFDIR) != S_IFDIR)
  {
    std::cerr << "ERROR: Expecting a directory. \"" << argv[1]
              << "\" is not a directory" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkNrrdSequenceReader> reader;
  reader->SetDirectoryName(argv[1]);
  reader->Update();

  // Cache all the volumes. Starting at 1, since the first one is already cached
  // by the previous Update call.
  for (int i = 1; i < reader->GetNumberOfNrrdFiles(); ++i)
  {
    std::cout << "Reading " << reader->GetCurrentFileName() << std::endl;
    reader->Next();
    reader->Update();
  }
  std::cout << "Reading " << reader->GetCurrentFileName() << std::endl;

  vtkImageData* im = reader->GetOutput();
  double* bounds = im->GetBounds();
  double depthRange[2] = { 0.0, 0.0 };
  depthRange[1] = vtkMath::Max(bounds[1], bounds[3]);
  depthRange[1] = vtkMath::Max(depthRange[1], bounds[5]);

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->ShadeOn();
  volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

  vtkDataArray* arr = im->GetPointData()->GetScalars();
  double range[2];
  arr->GetRange(range);

  // Prepare 1D Transfer Functions
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(depthRange[0], 1.0, 0.0, 0.0);
  ctf->AddRGBPoint(0.5 * (depthRange[0] + depthRange[1]), 0.5, 0.5, 0.5);
  ctf->AddRGBPoint(0.8 * (depthRange[0] + depthRange[1]), 0.5, 0.4, 0.6);
  ctf->AddRGBPoint(depthRange[1], 0.0, 1.0, 1.0);

  vtkNew<vtkPiecewiseFunction> pf;
  pf->AddPoint(0, 0.00);
  pf->AddPoint(50, 0.00);
  pf->AddPoint(165, 0.5);
  //pf->AddPoint(101, 0.01);
  pf->AddPoint(range[1], 0.8);

  volumeProperty->SetScalarOpacity(pf.GetPointer());
  volumeProperty->SetColor(ctf.GetPointer());

  vtkNew<vtkOpenGLGPUVolumeRayCastMapper> mapper;
  mapper->SetInputConnection(reader->GetOutputPort());
  mapper->SetUseJittering(1);
  // Tell the mapper to use the min and max of the color function nodes as the
  // lookup table range instead of the volume scalar range.
  mapper->SetColorRangeType(vtkGPUVolumeRayCastMapper::NATIVE);

  // Modify the shader to color based on the depth of the translucent voxel
  mapper->SetFragmentShaderCode(ColorCodedDepthFragmentShader);

  vtkNew<vtkVolume> volume;
  volume->SetMapper(mapper.GetPointer());
  volume->SetProperty(volumeProperty.GetPointer());

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetMultiSamples(0);
  renWin->SetSize(301, 300); // Intentional NPOT size

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());
  ren->AddVolume(volume.GetPointer());
  ren->ResetCamera();

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  vtkNew<ChangeSequenceStyle> style;
  style->Reader = reader.GetPointer();
  iren->SetInteractorStyle(style.GetPointer());

  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
