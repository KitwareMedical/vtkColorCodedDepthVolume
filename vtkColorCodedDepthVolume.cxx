/*=========================================================================

  Program:   Visualization Toolkit
  Module:    AnimateVTKVolume.cxx

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

// AnimateVTKVolume includes
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
    unsigned int numVolumes = this->Volumes->size();
    // Handle the next volume key
    if (key == "n")
    {
      this->PrevCurrent = this->Current;
      if (this->Current >= numVolumes - 1)
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
        this->Current = numVolumes - 1;
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
      for (int i = 0; i < numVolumes - 1; ++i)
      {
        this->PrevCurrent = this->Current;
        this->Current++;
        this->Update();
      }
    }
    else if (key == "a")
    {
      for (int i = 0; i < numVolumes; ++i)
      {
        vtkOpenGLGPUVolumeRayCastMapper* mapper =
          vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(
            this->Volumes->at(i)->GetMapper());
        mapper->ClearAllShaderReplacements();
        mapper->SetBlendModeToAverageIntensity();
        mapper->SetAverageIPScalarRange(80, 255);
      }
    }
    else if (key == "m")
    {
      for (int i = 0; i < numVolumes; ++i)
      {
        vtkOpenGLGPUVolumeRayCastMapper* mapper =
          vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(
            this->Volumes->at(i)->GetMapper());
        mapper->ClearAllShaderReplacements();
        mapper->SetBlendModeToMaximumIntensity();
      }
    }
    else if (key == "c")
    {
      for (int i = 0; i < numVolumes; ++i)
      {
        vtkOpenGLGPUVolumeRayCastMapper* mapper =
          vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(
            this->Volumes->at(i)->GetMapper());
        mapper->ClearAllShaderReplacements();
        mapper->SetColorRangeType(vtkGPUVolumeRayCastMapper::SCALAR);
        mapper->SetBlendModeToComposite();
      }
    }
    else if (key == "d")
    {
      for (int i = 0; i < numVolumes; ++i)
      {
        vtkOpenGLGPUVolumeRayCastMapper* mapper =
          vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(
            this->Volumes->at(i)->GetMapper());
        mapper->ClearAllShaderReplacements();
        mapper->SetColorRangeType(vtkGPUVolumeRayCastMapper::NATIVE);
        mapper->SetFragmentShaderCode(ColorCodedDepthFragmentShader);
      }
    }
    else if (key == "o")
    {
      for (int i = 0; i < numVolumes; ++i)
      {
        vtkOpenGLGPUVolumeRayCastMapper* mapper =
          vtkOpenGLGPUVolumeRayCastMapper::SafeDownCast(
            this->Volumes->at(i)->GetMapper());
        mapper->ClearAllShaderReplacements();
        mapper->SetBlendModeToComposite();
        mapper->SetColorRangeType(vtkGPUVolumeRayCastMapper::SCALAR);
        mapper->AddShaderReplacement(
          vtkShader::Fragment,
          "//VTK::Shading::Impl",
          true,
          "if (!g_skip)\n"
          "  {\n"
          "  vec4 scalar = texture3D(in_volume, g_dataPos);\n"
          "  scalar.r = scalar.r*in_volume_scale.r + in_volume_bias.r;\n"
          "  scalar = vec4(scalar.r,scalar.r,scalar.r,scalar.r);\n"
          "  g_srcColor = vec4(0.0);\n"
          "  g_srcColor.a = computeOpacity(scalar);\n"
          "  if (g_srcColor.a > 0.0)\n"
          "    {\n"
          "    g_srcColor = computeColor(scalar, g_srcColor.a);\n"
          "    // Opacity calculation using compositing:\n"
          "    // Here we use front to back compositing scheme whereby\n"
          "    // the current sample value is multiplied to the\n"
          "    // currently accumulated alpha and then this product\n"
          "    // is subtracted from the sample value to get the\n"
          "    // alpha from the previous steps. Next, this alpha is\n"
          "    // multiplied with the current sample colour\n"
          "    // and accumulated to the composited colour. The alpha\n"
          "    // value from the previous steps is then accumulated\n"
          "    // to the composited colour alpha.\n"
          "    vec3 l_dataPos = g_dataPos;\n"
          "    g_srcColor.rg *= (1 - l_dataPos.x)*0.8;\n"
          "    g_srcColor.b *= 0.7;\n"
          "    g_srcColor.rgb *= g_srcColor.a;\n"
          "    g_fragColor = (1.0f - g_fragColor.a) * g_srcColor + g_fragColor;\n"
          "    }\n"
          "  }"
          "",
          true
        );
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
  renWin->SetSize(401, 400); // Intentional NPOT size

  vtkNew<vtkRenderer> ren;
  renWin->AddRenderer(ren.GetPointer());

  vtkNew<vtkNrrdSequenceReader> reader;
  reader->SetDirectoryName(argv[1]);
  reader->Update();

  vtkImageData* im = reader->GetOutput();
  double* bounds = im->GetBounds();
  double depthRange[2] = { 0.0, 0.0 };
  depthRange[1] = bounds[5];

  vtkDataArray* arr = im->GetPointData()->GetScalars();
  double range[2];
  arr->GetRange(range);

  // Prepare 1D Transfer Functions
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  ctf->AddRGBPoint(80.0, 0.55, 0.45, 0.39);
  ctf->AddRGBPoint(112.0, 0.51, 0.4, 0.36);
  ctf->AddRGBPoint(176.0, 0.78, 0.54, 0.3);
  ctf->AddRGBPoint(255.0, 0.9, 0.9, 0.9);

  vtkNew<vtkPiecewiseFunction> pf;
  pf->AddPoint(0, 0.00);
  pf->AddPoint(80, 0.029);
  pf->AddPoint(112, 0.5);
  pf->AddPoint(176, 0.85);
  pf->AddPoint(255, 1.0);

  vtkNew<vtkPiecewiseFunction> gf;
  gf->AddPoint(80, 0.0);
  gf->AddPoint(112, 1.0);

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
  volumeProperty->SetScalarOpacity(pf.GetPointer());
  volumeProperty->SetColor(ctf.GetPointer());
  //  volumeProperty->SetGradientOpacity(gf.GetPointer());
  //  volumeProperty->SetDisableGradientOpacity(0);
  volumeProperty->ShadeOn();
  volumeProperty->SetDiffuse(0, 1);
  volumeProperty->SetAmbient(0, 0.6);
  volumeProperty->SetSpecular(0, 0.8);
  volumeProperty->SetSpecularPower(0, 50);

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
    mapper->SetUseJittering(1);
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
