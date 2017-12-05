cmake_minimum_required(VERSION 2.8)

PROJECT(vtkColorCodedDepthVolume)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

include(${VTK_CMAKE_DIR}/vtkEncodeString.cmake)
set(shader_files
  ColorCodedDepthFragmentShader.glsl
)
unset(shader_c_files)
foreach(file ${shader_files})
  vtk_encode_string(
    INPUT           "${file}"
    HEADER_OUTPUT   header
    SOURCE_OUTPUT   source
    )
  list(APPEND shader_c_files ${source})
endforeach()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_executable(${PROJECT_NAME} MACOSX_BUNDLE
  ${PROJECT_NAME}.cxx
  vtkNrrdSequenceReader.cxx
  vtkNrrdSequenceReader.h
  ${shader_c_files}
  )

if(VTK_LIBRARIES)
  target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} vtkHybrid vtkWidgets)
endif()
