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
#include <vtkCornerAnnotation.h>
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
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

// AnimateVolume includes
#include "vtkNrrdSequenceReader.h"
#include <ColorCodedDepthFragmentShader.h>

class ChangeSequenceStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static ChangeSequenceStyle* New();
  vtkTypeMacro(ChangeSequenceStyle, vtkInteractorStyleTrackballCamera);

  ChangeSequenceStyle()
  {
    PrevCurrent = 0;
    Current = 0;
  }

  virtual void Update()
  {
    double t = this->Renderer->GetLastRenderTimeInSeconds();
    std::cout << "FPS: " << 1 / t << std::endl;
    this->Volumes->at(this->PrevCurrent)->SetVisibility(0);
    this->Volumes->at(this->Current)->SetVisibility(1);
    this->Interactor->GetRenderWindow()->Render();
  }

  virtual void OnKeyPress() override
  {
    // Get the keypress
    vtkRenderWindowInteractor* rwi = this->Interactor;
    std::string key = rwi->GetKeySym();
    // Handle the next volume key
    if (key == "n")
    {
      this->PrevCurrent = this->Current;
      if (this->Current >= this->Volumes->size() - 1)
      {
        this->Current = 0;
      }
      else
      {
        this->Current++;
      }
    }
    else if (key == "p")
    {
      this->PrevCurrent = this->Current;
      if (this->Current <= 0)
      {
        this->Current = this->Volumes->size() - 1;
      }
      else
      {
        this->Current--;
      }
    }
    else if (key == "space")
    {
      this->PrevCurrent = this->Current;
      this->Current = 0;
      this->Update();
      for (int i = 0; i < this->Volumes->size() - 1; ++i)
      {
        this->PrevCurrent = this->Current;
        this->Current++;
        this->Update();
      }
    }
    this->Update();
    // Forward the event
    vtkInteractorStyleTrackballCamera::OnKeyPress();
  }

  vtkRenderer* Renderer;
  std::vector<vtkVolume*>* Volumes;
  int Current;
  int PrevCurrent;
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

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetMultiSamples(0);
  renWin->SetSize(801, 800); // Intentional NPOT size

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());

  vtkNew<vtkNrrdSequenceReader> reader;
  reader->SetDirectoryName(argv[1]);
  reader->Update();

  vtkImageData* im = reader->GetOutput();
  double* bounds = im->GetBounds();
  double depthRange[2] = { 0.0, 0.0 };
  depthRange[1] = bounds[5];

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

  vtkDataArray* arr = im->GetPointData()->GetScalars();
  double range[2];
  arr->GetRange(range);

  // Prepare 1D Transfer Functions
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(0.0, 0.88, 0.34, 0.34);
  ctf->AddRGBPoint(depthRange[1] / 7.0, 0.42, 0.0, 0.0);
  ctf->AddRGBPoint(2 * depthRange[1] / 7.0, 1.0, 0.38, 0.0);
  ctf->AddRGBPoint(3 * depthRange[1] / 7.0, 1.0, 1.0, 0.0);
  ctf->AddRGBPoint(4 * depthRange[1] / 7.0, 0.0, 0.5, 0.0);
  ctf->AddRGBPoint(5 * depthRange[1] / 7.0, 0.0, 1.0, 1.0);
  ctf->AddRGBPoint(6 * depthRange[1] / 7.0, 0.0, 0.0, 0.34);
  ctf->AddRGBPoint(depthRange[1], 0.27, 0.27, 0.85);

  vtkNew<vtkPiecewiseFunction> pf;
  pf->AddPoint(0, 0.00);
  pf->AddPoint(25, 0.0);
  pf->AddPoint(50, 0.1);
  pf->AddPoint(75, 0.0);
  pf->AddPoint(range[1], 0.0);

  volumeProperty->SetScalarOpacity(pf.GetPointer());
  volumeProperty->SetColor(ctf.GetPointer());

  std::vector<vtkVolume*> volumes;

  // Cache all the volumes. Starting at 1, since the first one is already cached
  // by the previous Update call.
  for (int i = 0; i < reader->GetNumberOfNrrdFiles(); ++i)
  {
    if (i > 0)
    {
      reader->Next();
      reader->Update();
    }
    std::cout << "Reading " << reader->GetCurrentFileName() << std::endl;
    vtkNew<vtkOpenGLGPUVolumeRayCastMapper> mapper;
    vtkNew<vtkImageData> im1;
    im1->DeepCopy(reader->GetOutput());
    mapper->SetInputData(im1.GetPointer());
    //mapper->SetInputConnection(reader->GetOutputPort());
    mapper->SetUseJittering(1);
    // Tell the mapper to use the min and max of the color function nodes as the
    // lookup table range instead of the volume scalar range.
    mapper->SetColorRangeType(vtkGPUVolumeRayCastMapper::NATIVE);

    // Modify the shader to color based on the depth of the translucent voxel
    mapper->SetFragmentShaderCode(ColorCodedDepthFragmentShader);

    vtkNew<vtkVolume> volume;
    volume->SetMapper(mapper.GetPointer());
    volume->SetProperty(volumeProperty.GetPointer());
    ren->AddVolume(volume.GetPointer());

    if (i > 0)
    {
      volume->SetVisibility(0);
    }

    volumes.push_back(volume.GetPointer());
  }

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin.GetPointer());

  vtkNew<ChangeSequenceStyle> style;
  style->Renderer = ren.GetPointer();
  style->Volumes = &volumes;
  iren->SetInteractorStyle(style.GetPointer());

  ren->ResetCamera();
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
