# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.19)

project("IGL" CXX C)

# cmake-format: off
option(IGL_WITH_SAMPLES   "Enable sample demo apps"            ON)

option(IGL_WITH_OPENGL    "Enable IGL/OpenGL"                  ON)
option(IGL_WITH_OPENGLES  "Enable IGL/OpenGL ES"              OFF)
option(IGL_WITH_VULKAN    "Enable IGL/Vulkan"                  ON)
option(IGL_WITH_METAL     "Enable IGL/Metal"                   ON)
option(IGL_WITH_WEBGL     "Enable IGL/WebGL"                  OFF)

option(IGL_WITH_IGLU      "Enable IGLU utils"                  ON)
option(IGL_WITH_SHELL     "Enable Shell utils"                 ON)
option(IGL_WITH_TESTS     "Enable IGL tests (gtest)"          OFF)
option(IGL_WITH_TRACY     "Enable Tracy profiler"             OFF)
option(IGL_WITH_TRACY_GPU "Enable Tracy profiler for the GPU" OFF)
option(IGL_WITH_OPENXR    "Enable OpenXR"                     OFF)
option(IGL_ENFORCE_LOGS   "Enable logs in Release builds"      ON)

option(IGL_DEPLOY_DEPS    "Deploy dependencies via CMake"      ON)

option(IGL_ENABLE_INSTALL "Enable install targets for IGL"    OFF)
# cmake-format: on

if(DEFINED ENV{VULKAN_SDK})
  message(STATUS "VULKAN_SDK=$ENV{VULKAN_SDK}")
  if(NOT EXISTS $ENV{VULKAN_SDK})
    message(FATAL_ERROR "$ENV{VULKAN_SDK} does not exist.")
  endif()
endif()

if(IOS)
  if(IGL_WITH_OPENGL)
    set(IGL_WITH_OPENGLES ON)
  endif()
  set(IGL_WITH_OPENGL OFF)
  set(IGL_WITH_VULKAN OFF)
  set(IGL_WITH_WEBGL OFF)
endif()

if(NOT APPLE)
  set(IGL_WITH_METAL OFF)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # disable for all targets due to warnings in third-party code
  add_definitions(-Wno-nullability-completeness)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>)
endif()

if(ANDROID)
  if(IGL_WITH_OPENGL)
    set(IGL_WITH_OPENGLES ON)
  endif()
  set(IGL_WITH_OPENGL OFF)
  set(IGL_WITH_VULKAN ON)
  set(IGL_WITH_WEBGL OFF)
elseif(UNIX)
  # disable for all targets due to warnings in third-party code
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-volatile>)
  add_definitions(-Wno-attributes)
endif()

if(EMSCRIPTEN)
  if(IGL_WITH_OPENXR)
    message(FATAL_ERROR "OpenXR is not supported on Emscripten.")
  endif()
  set(IGL_WITH_IGLU ON)
  set(IGL_WITH_OPENGL OFF)
  set(IGL_WITH_OPENGLES OFF)
  set(IGL_WITH_VULKAN OFF)
  set(IGL_WITH_WEBGL ON)
  set(IGL_WITH_SHELL OFF) # shell doesn't supported yet
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(igl_set_folder target folder_name)
  set_property(TARGET ${target} PROPERTY FOLDER ${folder_name})
endfunction()

function(igl_set_cxxstd target cpp_version)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD ${cpp_version})
  set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

# cmake-format: off
message(STATUS "IGL_WITH_SAMPLES  = ${IGL_WITH_SAMPLES}")

message(STATUS "IGL_WITH_OPENGL    = ${IGL_WITH_OPENGL}")
message(STATUS "IGL_WITH_OPENGLES  = ${IGL_WITH_OPENGLES}")
message(STATUS "IGL_WITH_VULKAN    = ${IGL_WITH_VULKAN}")
message(STATUS "IGL_WITH_METAL     = ${IGL_WITH_METAL}")
message(STATUS "IGL_WITH_WEBGL     = ${IGL_WITH_WEBGL}")

message(STATUS "IGL_WITH_IGLU      = ${IGL_WITH_IGLU}")
message(STATUS "IGL_WITH_SHELL     = ${IGL_WITH_SHELL}")
message(STATUS "IGL_WITH_TESTS     = ${IGL_WITH_TESTS}")
message(STATUS "IGL_WITH_TRACY     = ${IGL_WITH_TRACY}")
message(STATUS "IGL_WITH_TRACY_GPU = ${IGL_WITH_TRACY_GPU}")
message(STATUS "IGL_WITH_OPENXR    = ${IGL_WITH_OPENXR}")
message(STATUS "IGL_ENFORCE_LOGS   = ${IGL_ENFORCE_LOGS}")

message(STATUS "IGL_DEPLOY_DEPS    = ${IGL_DEPLOY_DEPS}")
# cmake-format: on

if(APPLE)
  if(IGL_WITH_OPENXR)
    message(FATAL_ERROR "OpenXR is not supported on Apple (yet).")
  endif()
  if(NOT (IGL_WITH_OPENGL OR IGL_WITH_VULKAN OR IGL_WITH_OPENGLES OR IGL_WITH_METAL OR IGL_WITH_WEBGL))
    message(FATAL_ERROR "At least one rendering backend should be defined (OpenGL, Vulkan or Metal).")
  endif()
else()
  if(NOT (IGL_WITH_OPENGL OR IGL_WITH_VULKAN OR IGL_WITH_OPENGLES OR IGL_WITH_WEBGL))
    message(FATAL_ERROR "At least one rendering backend should be defined (OpenGL or Vulkan).")
  endif()
endif()

if(IGL_WITH_SHELL AND (NOT IGL_WITH_IGLU))
  message(FATAL_ERROR "IGL_WITH_SHELL requires enabling IGL_WITH_IGLU.")
endif()

if(APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "IGL generator : " ${CMAKE_GENERATOR})
message(STATUS "IGL build type: " ${CMAKE_BUILD_TYPE})

set(IGL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IGL_DEPS_CACHE_FILE "${IGL_ROOT_DIR}/third-party/.bootstrap-deps.json")
find_package(Python3 COMPONENTS Interpreter)

message(STATUS "IGL_ROOT_DIR = " ${IGL_ROOT_DIR})

# Enable local "in-tree" builds
if(${CMAKE_CURRENT_SOURCE_DIR} MATCHES "^.*fbsource.*")
  get_filename_component(REPO_ROOT_DIR "../../../.." ABSOLUTE)
  message(STATUS "REPO_ROOT_DIR = " ${REPO_ROOT_DIR})
  include_directories(${REPO_ROOT_DIR})
endif()

# Enables multithreaded compilation and a conformant preprocessor on visual studio
if(MSVC)
  SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

if(IGL_DEPLOY_DEPS)
  # deploy dependencies during configuration
  execute_process(COMMAND ${Python3_EXECUTABLE} deploy_deps.py WORKING_DIRECTORY ${IGL_ROOT_DIR} COMMAND_ERROR_IS_FATAL ANY)

  if(NOT EXISTS ${IGL_DEPS_CACHE_FILE})
    message(FATAL_ERROR "Cannot deploy dependencies.")
  endif()

  # regenerate dependencies when .bootstrap-deps.json is older than bootstrap-deps.json
  add_custom_command(COMMAND ${Python3_EXECUTABLE} deploy_deps.py OUTPUT ${IGL_DEPS_CACHE_FILE}
                     DEPENDS "${IGL_ROOT_DIR}/third-party/bootstrap-deps.json" WORKING_DIRECTORY ${IGL_ROOT_DIR})
  add_custom_target(IGLDependencies DEPENDS ${IGL_DEPS_CACHE_FILE})

  igl_set_folder(IGLDependencies "IGL")
endif()

if (NOT IGL_WITH_TRACY AND IGL_WITH_TRACY_GPU)
  message(FATAL_ERROR "IGL_WITH_TRACY must be enabled to use Tracy's GPU profiling")
endif()


if(IGL_WITH_TRACY)
  add_definitions("-DTRACY_ENABLE=1")
  add_subdirectory(third-party/deps/src/tracy)
  igl_set_folder(TracyClient "third-party")
endif()

if(IGL_WITH_OPENXR)
  add_subdirectory(third-party/deps/src/openxr-sdk)
  igl_set_folder(openxr_loader "third-party/OpenXR")
endif()

add_subdirectory(src/igl)

if(IGL_WITH_TRACY)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_WITH_TRACY=1")
  if(IGL_WITH_TRACY_GPU)
    target_compile_definitions(IGLLibrary PUBLIC "IGL_WITH_TRACY_GPU=1")
  endif()
endif()

if(IGL_DEPLOY_DEPS)
  add_dependencies(IGLLibrary IGLDependencies)
endif()

if(IGL_ENFORCE_LOGS)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_FORCE_ENABLE_LOGS=1")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN)
  if(IGL_WITH_SAMPLES OR IGL_WITH_SHELL)
    target_compile_definitions(IGLLibrary PUBLIC "IGL_PLATFORM_LINUX_USE_EGL=0")
  else()
    target_compile_definitions(IGLLibrary PUBLIC "IGL_PLATFORM_LINUX_USE_EGL=1")
  endif()
endif()

# in the client code, use IGL_BACKEND_* macros instead of IGL_BACKEND_ENABLE_*
if(IGL_WITH_OPENGL)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_BACKEND_ENABLE_OPENGL=1")
endif()
if(IGL_WITH_OPENGLES OR IGL_WITH_WEBGL)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_BACKEND_ENABLE_OPENGL=1")
endif()
if(IGL_WITH_VULKAN)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_BACKEND_ENABLE_VULKAN=1")
endif()
if(IGL_WITH_IGLU)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_WITH_IGLU=1")
endif()
if(APPLE AND IGL_WITH_METAL)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_BACKEND_ENABLE_METAL=1")
endif()

target_compile_definitions(IGLLibrary PUBLIC "IGL_CMAKE_BUILD=1")

include_directories(.)

if(IGL_WITH_IGLU OR IGL_WITH_SAMPLES)
  add_library(IGLstb third-party/deps/patches/stb_impl/stb_image.c third-party/deps/patches/stb_impl/stb_image_resize.c
                     third-party/deps/patches/stb_impl/stb_image_write.c)
  target_include_directories(IGLstb PUBLIC "third-party/deps/src/stb")
  target_include_directories(IGLstb PUBLIC "third-party/deps/src")
  igl_set_folder(IGLstb "IGL")
endif()
if (IGL_WITH_IGLU)
  include_directories("third-party/deps/src/imgui")
  add_subdirectory(IGLU)
  if(IGL_WITH_SHELL)
    include_directories("third-party/deps/src/stb")
    add_subdirectory(shell)
  endif()
endif()

if(IGL_WITH_SAMPLES)
  if (EMSCRIPTEN)
    add_subdirectory(samples/wasm)
  else()
    include_directories("third-party/deps/src")
    include_directories("third-party/deps/src/bc7enc")
    include_directories("third-party/deps/src/gli")
    include_directories("third-party/deps/src/glm")
    include_directories("third-party/deps/src/stb")
    include_directories("third-party/deps/src/taskflow")
    include_directories("third-party/deps/src/3D-Graphics-Rendering-Cookbook")
    if(WIN32 OR (UNIX AND NOT APPLE AND NOT ANDROID))
      # cmake-format: off
      set(GLFW_BUILD_DOCS     OFF CACHE BOOL "")
      set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
      set(GLFW_BUILD_TESTS    OFF CACHE BOOL "")
      set(GLFW_INSTALL        OFF CACHE BOOL "")
      set(GLFW_VULKAN_STATIC  OFF CACHE BOOL "")
      # cmake-format: on
      add_subdirectory(third-party/deps/src/glfw)
      igl_set_folder(update_mappings "third-party/GLFW3")
    endif()
    if(UNIX AND NOT APPLE AND NOT ANDROID)
      find_package(OpenGL REQUIRED)
    endif()
    add_subdirectory(third-party/deps/src/bc7enc)
    igl_set_cxxstd(bc7enc 17)
    add_subdirectory(third-party/deps/src/meshoptimizer)
    add_subdirectory(third-party/deps/src/tinyobjloader)
    igl_set_folder(bc7enc "third-party")
    igl_set_folder(meshoptimizer "third-party")
    igl_set_folder(tinyobjloader "third-party/tinyobjloader")
    igl_set_folder(uninstall "third-party/tinyobjloader")
    if(NOT APPLE AND NOT ANDROID)
      add_subdirectory(samples/desktop)
      igl_set_folder(glfw "third-party/GLFW3")
    endif()
  endif()
endif()

if (IGL_WITH_VULKAN OR IGL_WITH_IGLU OR IGL_WITH_SAMPLES)
  add_subdirectory(third-party/deps/src/fmt "fmt")
  igl_set_folder(fmt "third-party")
endif()

if (IGL_WITH_IGLU OR IGL_WITH_SAMPLES)
  set(KTX_FEATURE_DOC OFF CACHE BOOL "")

  if (NOT EMSCRIPTEN)
    set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "")
  endif()

  set(KTX_FEATURE_JNI OFF CACHE BOOL "")
  set(KTX_FEATURE_KTX1 ON CACHE BOOL "")
  set(KTX_FEATURE_KTX2 ON CACHE BOOL "")
  set(KTX_FEATURE_LOADTEST_APPS OFF CACHE BOOL "")
  set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "")
  set(KTX_FEATURE_TESTS OFF CACHE BOOL "")
  set(KTX_FEATURE_TOOLS OFF CACHE BOOL "")
  set(KTX_FEATURE_VK_UPLOAD OFF CACHE BOOL "")
  add_subdirectory(third-party/deps/src/ktx-software)
  if(TARGET astcenc-avx2-static)
    igl_set_folder(astcenc-avx2-static "third-party/ktx-software")
  endif()
  igl_set_folder(ktx "third-party/ktx-software")
  igl_set_folder(ktx_read "third-party/ktx-software")
  igl_set_folder(ktx_version "third-party/ktx-software")
  igl_set_folder(obj_basisu_cbind "third-party/ktx-software")
  igl_set_folder(objUtil "third-party/ktx-software")
endif()


if(IGL_WITH_TRACY)
  target_link_libraries(IGLLibrary PUBLIC TracyClient)
endif()

if(IGL_WITH_OPENXR)
  target_compile_definitions(IGLLibrary PUBLIC "IGL_WITH_OPENXR=1")
  target_link_libraries(IGLLibrary PUBLIC OpenXR::openxr_loader)
  target_include_directories(IGLLibrary PUBLIC "${IGL_ROOT_DIR}/third-party/deps/src/openxr-sdk/include")
endif()

if(IGL_ENABLE_INSTALL)
  include(cmake/install.cmake)
endif()
