/*=============================================================================
Copyright and License information
=============================================================================*/

// NRRD includes
#include "vtkNrrdSequenceReader.h"

// VTK includes
#include <vtkDirectory.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkNrrdSequenceReader);

//-----------------------------------------------------------------------------
vtkNrrdSequenceReader::vtkNrrdSequenceReader()
{
  this->DirectoryName = nullptr;
  this->NumberOfNrrdFiles = 0;
}

//-----------------------------------------------------------------------------
vtkNrrdSequenceReader::~vtkNrrdSequenceReader()
{
}

//----------------------------------------------------------------------------
int vtkNrrdSequenceReader::RequestInformation(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  if (this->DirectoryName == nullptr)
  {
    if (this->FileName != nullptr)
    {
      return Superclass::RequestInformation(request, inputVector, outputVector);
    }
  }
  vtkDirectory* dir = vtkDirectory::New();
  int opened = dir->Open(this->DirectoryName);
  if (!opened)
  {
    vtkErrorMacro(<< "Failed to open directory: " << this->DirectoryName);
    dir->Delete();
    return 0;
  }
  dir->Delete();
  return 1;
}

//----------------------------------------------------------------------------
int vtkNrrdSequenceReader::RequestData(vtkInformation* request,
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  if (this->DirectoryName == nullptr)
  {
    if (this->FileName != nullptr)
    {
      return Superclass::RequestData(request, inputVector, outputVector);
    }
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkNrrdSequenceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  // os << indent << " = " << this-> << endl;
}
