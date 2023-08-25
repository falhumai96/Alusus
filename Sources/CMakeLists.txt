# This file is released under Alusus Public License, Version 1.0.
# For details on usage and copying conditions read the full license in the
# accompanying license file or at <https://alusus.org/license.html>.

project(Alusus)
cmake_minimum_required(VERSION 3.20.0)

# Build options.
option(ALUSUS_BIN_DIR_NAME "The Alusus \"bin\" directory name" "bin")
option(ALUSUS_LIB_DIR_NAME "The Alusus \"lib\" directory name" "lib")
option(ALUSUS_INCLUDE_DIR_NAME "The Alusus \"include\" directory name" "include")
option(ALUSUS_WIN32_UNICODE "Build Alusus for Windows with Unicode support" ON)
option(ALUSUS_USE_RPATH "Build Alusus with RPATH support for systems that support RPATH" OFF)

if (NOT WIN32)
  set(ALUSUS_WIN32_UNICODE OFF)
endif()

if(NOT DEFINED Python3_EXECUTABLE)
  # Only find Python executable once and cache it. Calling it twice will result in an
  # index out of bound error.
  #
  # E.g.:
  #
  # CMake Error at /usr/share/cmake-3.25/Modules/FindPython/Support.cmake:1887 (list):
  # list index: 10 out of range (-10, 9)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(Python3_EXECUTABLE "${Python3_EXECUTABLE}" CACHE STRING
    "Python 3 executable")
endif()

# Read version info.
execute_process(COMMAND ${Python3_EXECUTABLE} version_info.py
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/BuildSrc"
  OUTPUT_VARIABLE VersionInfo COMMAND_ERROR_IS_FATAL ANY)
string(REGEX MATCH "VERSION: ([0-9]+\\.[0-9]+\\.[0-9]+)" _dummy_ ${VersionInfo})
set(AlususVersion ${CMAKE_MATCH_1})
string(REGEX MATCH "^[0-9]+\\.[0-9]+" AlususShlibVersion ${AlususVersion})
string(REGEX MATCH "REVISION: ([A-Za-z0-9]+)" _dummy_ ${VersionInfo})
set(AlususRevision ${CMAKE_MATCH_1})
if (NOT ${AlususRevision} STREQUAL "")
  set(AlususRevision "-${AlususRevision}")
endif()
string(REGEX MATCH "DATE: ([0-9]+-[0-9]+-[0-9]+)" _dummy_ ${VersionInfo})
set(AlususReleaseDate ${CMAKE_MATCH_1})
string(REGEX MATCH "HIJRI_DATE: ([0-9]+-[0-9]+-[0-9]+)" _dummy_ ${VersionInfo})
set(AlususHijriReleaseDate ${CMAKE_MATCH_1})
message(STATUS "Alusus Version: " ${AlususVersion})
message(STATUS "Alusus Revision: " ${AlususRevision})
message(STATUS "Alusus Shlib Version: " ${AlususShlibVersion})
message(STATUS "Alusus Release Date: " ${AlususReleaseDate})
message(STATUS "Alusus Hijri Release Date: " ${AlususHijriReleaseDate})


# Detect the supported compiler used.
function(alusus_get_compiler_used COMPILER_TO_CHECK RETURN_VALUE_VAR RETURN_MSG_VAR)
  if(CMAKE_${COMPILER_TO_CHECK}_COMPILER_ID STREQUAL "MSVC")
    # Visual Studio MSVC compiler.
    set(${RETURN_VALUE_VAR} "Cl" PARENT_SCOPE)
    set(${RETURN_MSG_VAR} "MSVC Visual Studio compiler" PARENT_SCOPE)
  elseif(CMAKE_${COMPILER_TO_CHECK}_COMPILER_ID STREQUAL "GNU")
    # GNU GCC compiler.
    set(${RETURN_VALUE_VAR} "GCC" PARENT_SCOPE)
    set(${RETURN_MSG_VAR} "GNU compiler" PARENT_SCOPE)
  elseif(CMAKE_${COMPILER_TO_CHECK}_COMPILER_ID STREQUAL "AppleClang")
    # Apple Clang compiler.
    set(${RETURN_VALUE_VAR} "AppleClang" PARENT_SCOPE)
    set(${RETURN_MSG_VAR} "Darwin LLVM/Clang compiler" PARENT_SCOPE)
  elseif(CMAKE_${COMPILER_TO_CHECK}_COMPILER_ID STREQUAL "Clang")
    if(NOT DEFINED CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT OR
      (DEFINED CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT AND CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT STREQUAL "GNU") OR
      (DEFINED CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT AND CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT STREQUAL ""))
      if(NOT DEFINED CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID OR
        (DEFINED CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID AND CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID STREQUAL "GNU") OR
        (DEFINED CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID AND CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID STREQUAL ""))
        # GNU Clang compiler.
        set(${RETURN_VALUE_VAR} "GNUClang" PARENT_SCOPE)
        set(${RETURN_MSG_VAR} "LLVM/Clang compiler with GNU ABI" PARENT_SCOPE)
      elseif(DEFINED CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID AND CMAKE_${COMPILER_TO_CHECK}_SIMULATE_ID STREQUAL "MSVC")
        # MSVC Clang compiler with GNU frontend.
        set(${RETURN_VALUE_VAR} "MSVCClang" PARENT_SCOPE)
        set(${RETURN_MSG_VAR} "LLVM/Clang compiler with MSVC ABI and GNU interface" PARENT_SCOPE)
      else()
        message(FATAL_ERROR "Unknown ${COMPILER_TO_CHECK} compiler.")
      endif()
    elseif(DEFINED CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT AND CMAKE_${COMPILER_TO_CHECK}_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      # MSVC Clang compiler with MSVC frontend. It will be treated like the Visual Studio MSVC compiler.
      set(${RETURN_VALUE_VAR} "Cl" PARENT_SCOPE)
      set(${RETURN_MSG_VAR} "LLVM/Clang compiler with MSVC ABI and MSVC interface" PARENT_SCOPE)
    else()
      message(FATAL_ERROR "Unknown ${COMPILER_TO_CHECK} compiler.")
    endif()
  else()
    message(FATAL_ERROR "Unknown ${COMPILER_TO_CHECK} compiler.")
  endif()
endfunction()


alusus_get_compiler_used("CXX" AlususCompilerUsed AlususCompilerUsedMsg)
message(STATUS "Alusus compiler used: ${AlususCompilerUsedMsg}")

# Generate Alusus definitions.
execute_process(COMMAND
  "${Python3_EXECUTABLE}" generate_defs_header.py
  "${CMAKE_BINARY_DIR}/AlususDefs"
  "${AlususVersion}"
  "${AlususRevision}"
  "${AlususShlibVersion}"
  "${AlususReleaseDate}"
  "${AlususHijriReleaseDate}"
  "${AlususCompilerUsed}"
  "${CMAKE_BUILD_TYPE}"
  "${CMAKE_SYSTEM_NAME}"
  "${CMAKE_SYSTEM_PROCESSOR}"
  "${ALUSUS_BIN_DIR_NAME}"
  "${ALUSUS_LIB_DIR_NAME}"
  "${ALUSUS_INCLUDE_DIR_NAME}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/BuildSrc" COMMAND_ERROR_IS_FATAL ANY)

# Prepare compile and link flags based on the compiler selected.
if(AlususCompilerUsed STREQUAL "GNUClang" OR AlususCompilerUsed STREQUAL "MSVCClang" OR AlususCompilerUsed STREQUAL "AppleClang")
  # Clang with GNU interface options.
  set(Alusus_COMPILE_FLAGS "${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} -Werror=return-type -Wunused-variable -Winit-self -Wuninitialized -Wunused-const-variable -Wno-unused-result -fno-rtti -fvisibility=hidden")
elseif (AlususCompilerUsed STREQUAL "GCC")
  # GCC with GNU interface options.
  set(Alusus_COMPILE_FLAGS "${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} -Werror=return-type -Wunused-variable -Winit-self -Wuninitialized -Wunused-but-set-parameter -Wunused-but-set-variable -Wno-unused-result -fno-rtti -fvisibility=hidden")
elseif(AlususCompilerUsed STREQUAL "Cl")
  # MSVC Cl or Clang with MSVC Cl interface options.
  set(Alusus_COMPILE_FLAGS "${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} /w44265 /w44130 /w44296 /wd4100 /wd4101 /wd4189 /EHsc /wd4005 /wd4146 /wd4355 /wd4800 /wd4996")  
endif()

if(ALUSUS_USE_RPATH)
  set(CMAKE_SKIP_BUILD_RPATH OFF)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH ON)
  endif()
endif()

# Clear CMAKE_CXX_FLAGS_* because we are going to provide our own compilation
# flags here.
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS "")

# Find Alusus direct dependencies.
find_package(nowide CONFIG REQUIRED)
find_package(LLVM CONFIG REQUIRED)

# Macro for generating pre-compiled headers.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${Alusus_SOURCE_DIR}")

# Enable testing
enable_testing()

set (CUSTOM_OUTPUT_PATH "" CACHE PATH "Custom path to output all executable and library files. Useful during development.")
if (NOT CUSTOM_OUTPUT_PATH STREQUAL "")
  if (NOT EXISTS "${CUSTOM_OUTPUT_PATH}")
    message(SEND_ERROR "Invalid custom output path was provided: " ${CUSTOM_OUTPUT_PATH} ".")
  else()
    message(STATUS "The following custom output path was provided: " ${CUSTOM_OUTPUT_PATH} ".")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CUSTOM_OUTPUT_PATH}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CUSTOM_OUTPUT_PATH}")
  endif()
endif()

# Add projects
add_subdirectory(AlususOSAL)
add_subdirectory(Srt)
add_subdirectory(Core)
add_subdirectory(Spp)
add_subdirectory(Tests)
