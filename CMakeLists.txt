cmake_minimum_required(VERSION 3.24)

project(PhantomMirror VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "Phantom Mirror is Windows-only because Spout2 and layered overlay rendering use Win32 APIs.")
endif()

set(PHANTOM_MIRROR_UPDATE_REPO "" CACHE STRING "GitHub owner/repo used for in-app update checks.")
set(PHANTOM_MIRROR_UPDATE_ASSET "Phantom-Mirror-windows-x64.zip" CACHE STRING "Release asset name used for in-app updates.")
set(PHANTOM_MIRROR_APP_VERSION "${PROJECT_VERSION}" CACHE STRING "Application version used for in-app update comparison.")
set(PHANTOM_MIRROR_NDI_RUNTIME_DIR "" CACHE PATH "Optional path to an installed NDI runtime directory containing Processing.NDI.Lib.x64.dll.")
option(PHANTOM_MIRROR_BUNDLE_NDI_RUNTIME "Copy and install the proprietary NDI runtime beside the app." OFF)

find_package(Qt6 REQUIRED COMPONENTS Network Widgets)

if(DEFINED QT6_IS_SHARED_LIBS AND NOT QT6_IS_SHARED_LIBS)
  message(FATAL_ERROR "Phantom Mirror must use shared Qt libraries for LGPL-compatible binary distribution.")
endif()

if(DEFINED QT_FEATURE_shared AND NOT QT_FEATURE_shared)
  message(FATAL_ERROR "Phantom Mirror must use shared Qt libraries for LGPL-compatible binary distribution.")
endif()

get_target_property(PHANTOM_MIRROR_QT_CORE_TYPE Qt6::Core TYPE)
if(PHANTOM_MIRROR_QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
  message(FATAL_ERROR "Static Qt was detected. Install/use the shared MSVC Qt build instead.")
endif()

qt_standard_project_setup()

set(SPOUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Spout2/SPOUTSDK")
set(SPOUT_GL "${SPOUT_ROOT}/SpoutGL")
set(SPOUT_DX "${SPOUT_ROOT}/SpoutDirectX/SpoutDX")

set_source_files_properties(${SPOUT_GL}/SpoutFrameCount.cpp PROPERTIES
  COMPILE_OPTIONS "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/native/spout_winmm_compat.h"
)

qt_add_executable(PhantomMirror WIN32
  resources/resources.qrc
  resources/app.rc
  src/app/arrow_combo_box.cpp
  src/app/arrow_combo_box.h
  src/app/app_config.cpp
  src/app/app_config.h
  src/app/app_icon.cpp
  src/app/app_icon.h
  src/app/app_strings.cpp
  src/app/app_strings.h
  src/app/hotkey_edit.cpp
  src/app/hotkey_edit.h
  src/app/main.cpp
  src/app/main_window.cpp
  src/app/main_window.h
  src/app/ndi_controller.cpp
  src/app/ndi_controller.h
  src/app/onboarding_dialog.cpp
  src/app/onboarding_dialog.h
  src/app/settings_dialog.cpp
  src/app/settings_dialog.h
  src/app/toggle_switch.cpp
  src/app/toggle_switch.h
  src/app/spout_controller.cpp
  src/app/spout_controller.h
  src/app/updater.cpp
  src/app/updater.h
  src/native/ndi_overlay.cpp
  src/native/ndi_overlay.h
  src/native/ndi_sdk.cpp
  src/native/ndi_sdk.h
  src/native/spout_overlay.cpp
  src/native/spout_overlay.h
  ${SPOUT_DX}/SpoutDX.cpp
  ${SPOUT_GL}/SpoutDirectX.cpp
  ${SPOUT_GL}/SpoutSenderNames.cpp
  ${SPOUT_GL}/SpoutFrameCount.cpp
  ${SPOUT_GL}/SpoutCopy.cpp
  ${SPOUT_GL}/SpoutSharedMemory.cpp
  ${SPOUT_GL}/SpoutUtils.cpp
)

target_include_directories(PhantomMirror PRIVATE
  src/native
  ${SPOUT_DX}
  ${SPOUT_GL}
)

target_compile_definitions(PhantomMirror PRIVATE
  NOMINMAX
  WIN32_LEAN_AND_MEAN
  PHANTOM_MIRROR_VERSION="${PHANTOM_MIRROR_APP_VERSION}"
  PHANTOM_MIRROR_UPDATE_REPO="${PHANTOM_MIRROR_UPDATE_REPO}"
  PHANTOM_MIRROR_UPDATE_ASSET="${PHANTOM_MIRROR_UPDATE_ASSET}"
  UNICODE
  _UNICODE
)

target_link_libraries(PhantomMirror PRIVATE
  Qt6::Network
  Qt6::Widgets
  d3d11
  dxgi
  user32
  gdi32
  msimg32
  psapi
  winmm
)

set_target_properties(PhantomMirror PROPERTIES
  OUTPUT_NAME "Phantom Mirror"
)

if(PHANTOM_MIRROR_BUNDLE_NDI_RUNTIME)
  if(NOT PHANTOM_MIRROR_NDI_RUNTIME_DIR)
    foreach(_candidate
        "$ENV{NDI_RUNTIME_DIR_V6}"
        "$ENV{NDI_RUNTIME_DIR_V5}"
        "C:/Program Files/NDI/NDI 6 Tools/Runtime"
        "C:/Program Files/NDI/NDI 5 Runtime/v5")
      if(EXISTS "${_candidate}/Processing.NDI.Lib.x64.dll")
        set(PHANTOM_MIRROR_NDI_RUNTIME_DIR "${_candidate}")
        break()
      endif()
    endforeach()
  endif()

  if(NOT EXISTS "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.x64.dll")
    message(FATAL_ERROR "NDI bundling was enabled, but the runtime was not found. Set PHANTOM_MIRROR_NDI_RUNTIME_DIR to a folder containing Processing.NDI.Lib.x64.dll.")
  endif()

  add_custom_command(TARGET PhantomMirror POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.x64.dll"
      "$<TARGET_FILE_DIR:PhantomMirror>/Processing.NDI.Lib.x64.dll"
  )

  if(EXISTS "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.Licenses.txt")
    add_custom_command(TARGET PhantomMirror POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.Licenses.txt"
        "$<TARGET_FILE_DIR:PhantomMirror>/Processing.NDI.Lib.Licenses.txt"
    )
  endif()
endif()

install(TARGETS PhantomMirror
  BUNDLE DESTINATION .
  RUNTIME DESTINATION .
)

if(PHANTOM_MIRROR_BUNDLE_NDI_RUNTIME)
  install(FILES "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.x64.dll"
    DESTINATION .
  )

  if(EXISTS "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.Licenses.txt")
    install(FILES "${PHANTOM_MIRROR_NDI_RUNTIME_DIR}/Processing.NDI.Lib.Licenses.txt"
      DESTINATION .
    )
  endif()
endif()
