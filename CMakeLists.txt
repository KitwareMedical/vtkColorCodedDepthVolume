cmake_minimum_required(VERSION 2.8)

PROJECT(AnimateVolume)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(${PROJECT_NAME} MACOSX_BUNDLE
  ${PROJECT_NAME}.cxx
  vtkNrrdSequenceReader.cxx
  vtkNrrdSequenceReader.h
  )

if(VTK_LIBRARIES)
  target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} vtkHybrid vtkWidgets)
endif()
