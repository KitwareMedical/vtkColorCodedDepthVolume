cmake_minimum_required(VERSION 2.8)

PROJECT(AnimateVolume)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(shader_files
  ColorCodedDepthFragmentShader.glsl
)
unset(shader_c_files)
foreach(file ${shader_files})
  get_filename_component(file_we ${file} NAME_WE)
  set(src  ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  set(res  ${CMAKE_CURRENT_BINARY_DIR}/${file_we}.cxx)
  set(resh ${CMAKE_CURRENT_BINARY_DIR}/${file_we}.h)
  list(APPEND shader_c_files ${res})
  add_custom_command(
    OUTPUT ${res} ${resh}
    DEPENDS ${src} vtkEncodeString
    COMMAND vtkEncodeString
    ARGS ${res} ${src} ${file_we}
      --build-header
    )
endforeach()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

add_executable(${PROJECT_NAME} MACOSX_BUNDLE
  ${PROJECT_NAME}.cxx
  vtkNrrdSequenceReader.cxx
  vtkNrrdSequenceReader.h
  VolumeAnimator.cxx
  VolumeAnimator.h
  AnimationCueObserver.cxx
  AnimationCueObserver.h
  ${shader_c_files}
  )

if(VTK_LIBRARIES)
  target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} vtkHybrid vtkWidgets)
endif()
