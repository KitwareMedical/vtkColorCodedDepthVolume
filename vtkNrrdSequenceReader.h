/*=============================================================================
Copyright and License information
=============================================================================*/

#ifndef vtkNrrdSequenceReader_h
#define vtkNrrdSequenceReader_h

// VTK includes
#include <vtkNrrdReader.h>

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
  int NumberOfNrrdFiles;

private:
  vtkNrrdSequenceReader(const vtkNrrdSequenceReader&) = delete;
  void operator=(const vtkNrrdSequenceReader) = delete;
};

#endif // vtkNrrdSequenceReader_h
