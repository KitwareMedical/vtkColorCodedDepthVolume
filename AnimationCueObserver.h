/*=============================================================================
Copyright and License information
=============================================================================*/

#ifndef AnimationCueObserver_h
#define AnimationCueObserver_h

// VTK includes
#include <vtkCommand.h>

// Forward declarations
class VolumeAnimator;

class AnimationCueObserver : public vtkCommand
{
public:
  static AnimationCueObserver* New();

  virtual void Execute(vtkObject* caller, unsigned long event, void* callData);

  VolumeAnimator* Animator;

protected:
  AnimationCueObserver();
  ~AnimationCueObserver();
};

#endif // AnimationCueObserver_h
