#ifndef VolumeAnimator_h
#define VolumeAnimator_h

// VTK includes
#include <vtkAnimationCue.h>

// Forward declarations
class AnimationCueObserver;
class vtkNrrdSequenceReader;
class vtkRenderWindow;

class VolumeAnimator
{
public:
  VolumeAnimator();
  ~VolumeAnimator();

  virtual void SetReader(vtkNrrdSequenceReader* r);
  virtual void SetRenderWindow(vtkRenderWindow* r);
  virtual void AddObserversToCue(vtkAnimationCue* cue);
  virtual void Start(vtkAnimationCue::AnimationCueInfo* info);
  virtual void Tick(vtkAnimationCue::AnimationCueInfo* info);
  virtual void End(vtkAnimationCue::AnimationCueInfo* info);

protected:
  AnimationCueObserver* Observer;
  vtkNrrdSequenceReader* Reader;
  vtkRenderWindow* RenderWindow;
};

#endif // VolumeAnimator_h
