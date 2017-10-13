/*=============================================================================
Copyright and License information
=============================================================================*/

// AnimateVTKVolume includes
#include "AnimationCueObserver.h"
#include "VolumeAnimator.h"

// VTK includes
#include <vtkAnimationCue.h>

//-----------------------------------------------------------------------------
AnimationCueObserver* AnimationCueObserver::New()
{
  return new AnimationCueObserver;
}

//-----------------------------------------------------------------------------
AnimationCueObserver::AnimationCueObserver()
{
  this->Animator = nullptr;
}

//-----------------------------------------------------------------------------
AnimationCueObserver::~AnimationCueObserver()
{
}

//----------------------------------------------------------------------------
void AnimationCueObserver::Execute(vtkObject* vtkNotUsed(caller),
                                   unsigned long event,
                                   void* callData)
{
  if (this->Animator)
  {
    vtkAnimationCue::AnimationCueInfo* info =
      static_cast<vtkAnimationCue::AnimationCueInfo*>(callData);
    switch (event)
    {
      case vtkCommand::StartAnimationCueEvent:
        this->Animator->Start(info);
        break;
      case vtkCommand::EndAnimationCueEvent:
        this->Animator->End(info);
        break;
      case vtkCommand::AnimationCueTickEvent:
        this->Animator->Tick(info);
        break;
    }
  }
}
