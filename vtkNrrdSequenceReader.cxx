/*=============================================================================
Copyright and License information
=============================================================================*/

// NRRD includes
#include "vtkNrrdSequenceReader.h"

// VTK includes
#include <vtkDirectory.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkNrrdSequenceReader);

//-----------------------------------------------------------------------------
vtkNrrdSequenceReader::vtkNrrdSequenceReader()
{
  this->DirectoryName = nullptr;
  this->NumberOfNrrdFiles = 0;
  this->CurrentIndex = 0;
}

//-----------------------------------------------------------------------------
vtkNrrdSequenceReader::~vtkNrrdSequenceReader()
{
  std::map<std::string, vtkImageData*>::iterator it;
  for (it = this->NrrdVolumes.begin(); it != this->NrrdVolumes.end(); ++it)
  {
    if (it->second)
    {
      it->second->Delete();
    }
  }
  this->NrrdVolumes.clear();
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
  vtkIdType numFiles = dir->GetNumberOfFiles();
  for (vtkIdType i = 0; i < numFiles; i++)
  {
    if (strcmp(dir->GetFile(i), ".") == 0 || strcmp(dir->GetFile(i), "..") == 0)
    {
      continue;
    }

    std::string fileString = this->DirectoryName;
    fileString += "/";
    fileString += dir->GetFile(i);

    int val = this->CanReadFile(fileString.c_str());

    if (val == 2)
    {
      vtkDebugMacro(<< "Adding " << fileString.c_str() << " to NrrdFileNames.");
      this->NrrdFileNames.insert(dir->GetFile(i));
    }
    else
    {
      vtkDebugMacro(<< fileString.c_str()
                    << " - vtkNrrdReader CanReadFile returned : "
                    << val);
    }
  }
  this->NumberOfNrrdFiles = this->NrrdFileNames.size();
  dir->Delete();

  std::string fstring(this->GetCurrentFileName());
  std::string currentFileString = this->DirectoryName;
  currentFileString += "/";
  currentFileString += fstring;
  // Find if we have cached copy of the data already
  std::map<std::string, vtkImageData*>::const_iterator mit =
    this->NrrdVolumes.find(fstring);
  if (mit == this->NrrdVolumes.end())
  {
    // If we have a cached copy, no need to invoke superclass methods
    this->SetFileName(currentFileString.c_str());
    return this->Superclass::RequestInformation(
      request, inputVector, outputVector);
  }
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
      return this->Superclass::RequestData(request, inputVector, outputVector);
    }
  }
  if (this->GetNumberOfNrrdFiles() < 1)
  {
    // Nothing to do
    return 1;
  }

  // Some VTK output management
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* output =
    vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  std::string fileString(this->GetCurrentFileName());

  int result = 0;

  // Find if we have cached copy of the data already
  std::map<std::string, vtkImageData*>::const_iterator mit =
    this->NrrdVolumes.find(fileString);
  if (mit != this->NrrdVolumes.end())
  {
    vtkDebugMacro(<<"Loading cached copy of " << fileString);
    // Use the cached copy
    output->ShallowCopy(mit->second);
    result = 1;
  }
  else
  {
    vtkDebugMacro( << "Reading " << fileString);
    result = this->Superclass::RequestData(request, inputVector, outputVector);

    // Once the superclass is done creating the data, cache it for future.
    std::pair<std::string, vtkImageData*> outputCache;
    vtkImageData* im = vtkImageData::New();
    im->DeepCopy(output);
    outputCache.first = fileString;
    outputCache.second = im;
    this->NrrdVolumes.insert(outputCache);
  }

  return result;
}

//----------------------------------------------------------------------------
std::string vtkNrrdSequenceReader::GetCurrentFileName()
{
  std::set<std::string>::const_iterator it = this->NrrdFileNames.begin();
  this->CurrentIndex = this->CurrentIndex < 0 ? 0 : this->CurrentIndex;
  this->CurrentIndex = this->CurrentIndex >= this->NumberOfNrrdFiles
    ? this->NumberOfNrrdFiles - 1
    : this->CurrentIndex;
  std::advance(it, this->CurrentIndex);
  return *it;
}

//----------------------------------------------------------------------------
void vtkNrrdSequenceReader::PrintSelf(ostream& os, vtkIndent indent)
{
  // os << indent << " = " << this-> << endl;
}
