// AnimateVTKVolume includes
#include "VolumeAnimator.h"
#include "AnimationCueObserver.h"
#include "vtkNrrdSequenceReader.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkRenderWindow.h>

//----------------------------------------------------------------------------
VolumeAnimator::VolumeAnimator()
{
  this->Reader = nullptr;
  this->RenderWindow = nullptr;
  this->Observer = AnimationCueObserver::New();
  this->Observer->Animator = this;
}

//----------------------------------------------------------------------------
VolumeAnimator::~VolumeAnimator()
{
  if (this->Reader != nullptr)
  {
    this->Reader->UnRegister(0);
    this->Reader = nullptr;
  }
  if (this->RenderWindow != nullptr)
  {
    this->RenderWindow->UnRegister(0);
    this->RenderWindow = nullptr;
  }
  this->Observer->UnRegister(0);
}

//----------------------------------------------------------------------------
void VolumeAnimator::SetReader(vtkNrrdSequenceReader* r)
{
  if (this->Reader != nullptr)
  {
    this->Reader->UnRegister(0);
  }
  this->Reader = r;
  this->Reader->Register(0);
}

//----------------------------------------------------------------------------
void VolumeAnimator::SetRenderWindow(vtkRenderWindow* r)
{
  if (this->RenderWindow)
  {
    this->RenderWindow->UnRegister(0);
  }
  this->RenderWindow = r;
  this->RenderWindow->Register(0);
}

//----------------------------------------------------------------------------
void VolumeAnimator::AddObserversToCue(vtkAnimationCue* cue)
{
  cue->AddObserver(vtkCommand::StartAnimationCueEvent, this->Observer);
  cue->AddObserver(vtkCommand::EndAnimationCueEvent, this->Observer);
  cue->AddObserver(vtkCommand::AnimationCueTickEvent, this->Observer);
}

//----------------------------------------------------------------------------
void VolumeAnimator::Start(vtkAnimationCue::AnimationCueInfo* info)
{
  if (this->Reader)
  {
    this->Reader->SetCurrentIndex(0);
  }
}

//----------------------------------------------------------------------------
void VolumeAnimator::End(vtkAnimationCue::AnimationCueInfo* info)
{
  if (this->Reader)
  {
    this->Reader->SetCurrentIndex(this->Reader->GetNumberOfNrrdFiles() - 1);
  }
}

//----------------------------------------------------------------------------
void VolumeAnimator::Tick(vtkAnimationCue::AnimationCueInfo* info)
{
  if (this->Reader)
  {
    this->Reader->Next();
  }
  if (this->RenderWindow)
  {
    this->RenderWindow->Render();
  }
}
