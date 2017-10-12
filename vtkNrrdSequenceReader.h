/*=============================================================================
Copyright and License information
=============================================================================*/

#ifndef vtkNrrdSequenceReader_h
#define vtkNrrdSequenceReader_h

// VTK includes
#include <vtkNrrdReader.h>

// STL includes
#include <map>
#include <set>
#include <string>

// Forward declarations
class vtkImageData;

/// VTK reader for a sequence of Nrrd volumes (time steps)
class vtkNrrdSequenceReader : public vtkNrrdReader
{
public:
  vtkTypeMacro(vtkNrrdSequenceReader, vtkNrrdReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkNrrdSequenceReader* New();

  //@{
  /*
   * Set / Get the directory name
   */
  vtkSetStringMacro(DirectoryName);
  vtkGetStringMacro(DirectoryName);
  //@}

  /*
   * Get number of Nrrd files in the directory
   */
  vtkGetMacro(NumberOfNrrdFiles, int);

  //@{
  /*
   * Set/Get the current volume index.
   * If the files in the directory are numbered, this index corresponds to the
   * file index.
   */
  vtkSetMacro(CurrentIndex, int);
  vtkGetMacro(CurrentIndex, int);
  virtual void Next()
  {
    this->SetCurrentIndex(CurrentIndex + 1);
  }
  virtual void Previous()
  {
    this->SetCurrentIndex(this->CurrentIndex - 1);
  }
  //@}

protected:
  vtkNrrdSequenceReader();
  ~vtkNrrdSequenceReader();

  int RequestInformation(vtkInformation* request,
                         vtkInformationVector** inputVector,
                         vtkInformationVector* outputVector) override;

  int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector) override;

  char* DirectoryName;
  int CurrentIndex;
  int NumberOfNrrdFiles;
  std::map<std::string, vtkImageData*> NrrdVolumes;
  std::set<std::string> NrrdFileNames;

private:
  vtkNrrdSequenceReader(const vtkNrrdSequenceReader&) = delete;
  void operator=(const vtkNrrdSequenceReader) = delete;
};

#endif // vtkNrrdSequenceReader_h
