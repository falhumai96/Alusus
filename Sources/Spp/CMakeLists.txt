# Copyright (C) 2022 Sarmad Khalid Abdullah
#
# This file is released under Alusus Public License, Version 1.0.
# For details on usage and copying conditions read the full license in the
# accompanying license file or at <https://alusus.org/license.html>.

project(AlususSpp)
cmake_minimum_required(VERSION 3.20.0)

# Header and library files paths for LLVM
include_directories("${LLVM_INCLUDE_DIRS}")
link_directories("${LLVM_LIBRARY_DIRS}")
add_definitions(${LLVM_DEFINITIONS})

# Header and library files paths for AlususSrl
include_directories("${AlususSrl_SOURCE_DIR}")

# Header and library files paths for AlususCore
include_directories("${AlususCore_SOURCE_DIR}")

# Let's suppose we want to build a JIT compiler with support for
# binary code (no interpreter):
execute_process(COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-config --libs core mcjit orcjit x86 aarch64 arm powerpc systemz webassembly
                OUTPUT_VARIABLE REQ_LLVM_LIBRARIES COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-config --system-libs
                OUTPUT_VARIABLE REQ_SYSTEM_LIBRARIES COMMAND_ERROR_IS_FATAL ANY)
if ((AlususCompilerUsed STREQUAL "GNUClang" OR AlususCompilerUsed STREQUAL "AppleClang") AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Replace -llibxml2.tbd with -lxml2 in REQ_SYSTEM_LIBRARIES to fix issue in macOS.
  # TODO: Remove this hack after moving to a newer version of LLVM that fixes it.
  string(REPLACE "-llibxml2.tbd" "-lxml2" REQ_SYSTEM_LIBRARIES ${REQ_SYSTEM_LIBRARIES})
endif()
# Make sure the compiler finds the source files.
include_directories("${AlususSpp_SOURCE_DIR}")

# Create the list of files to be added to the project.
set(AlususSpp_SOURCE_SUBDIRS Ast Notices Handlers Grammar CodeGen LlvmCodeGen Rt)
set(AlususSpp_SOURCE_FILES
  spp.h
  LibraryGateway.h
  LibraryGateway.cpp
  ExecutionContext.h
  BuildManager.h
  BuildManager.cpp
  SeekerExtension.h
  SeekerExtension.cpp
  RootScopeHandlerExtension.h
  RootScopeHandlerExtension.cpp
  RootManagerExtension.h
  RootManagerExtension.cpp
  main.cpp
  )
foreach (dir ${AlususSpp_SOURCE_SUBDIRS})
  file(GLOB headers ${dir}/*.h)
  file(GLOB sources ${dir}/*.cpp)
  # Place files in folders for IDEs (only tested with  Visual Studio and Eclipse at the moment.)
  source_group("HeaderFiles\\${dir}" FILES ${headers})
  source_group("SourceFiles\\${dir}" FILES ${sources})
  set(AlususSpp_SOURCE_FILES ${AlususSpp_SOURCE_FILES} ${sources} ${headers})
endforeach(dir)

# Create AlususSpp library project.
add_library(AlususSpp SHARED ${AlususSpp_SOURCE_FILES})
set_target_properties(AlususSpp PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  OUTPUT_NAME alusus_spp
  DEBUG_OUTPUT_NAME alusus_spp.dbg
  VERSION ${AlususVersion}
  SHLIBVERSION ${AlususShlibVersion}
)
if(ALUSUS_USE_RPATH)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(AlususSpp PROPERTIES
      INSTALL_RPATH "@executable_path;@executable_path/../${ALUSUS_LIB_DIR_NAME}/"
    )
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(AlususSpp PROPERTIES
      INSTALL_RPATH "$ORIGIN;$ORIGIN/../${ALUSUS_LIB_DIR_NAME}/"
    )
  endif()
endif()
target_precompile_headers(AlususSpp PRIVATE "spp.h")
target_include_directories(AlususSpp PRIVATE "${CMAKE_BINARY_DIR}/AlususDefs")
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususSpp PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()

# Finally, we link LLVM and AlususCore libraries to our library.
target_link_libraries(AlususSpp PRIVATE ${REQ_LLVM_LIBRARIES} ${REQ_SYSTEM_LIBRARIES} AlususSrlLib AlususCoreLib AlususStorage AlususOSAL ${CMAKE_DL_LIBS} nowide::nowide)

# Prevent the exporting of LLVM symbols
if ((AlususCompilerUsed STREQUAL "GNUClang" OR AlususCompilerUsed STREQUAL "AppleClang") AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(AlususSpp PRIVATE -Wl,-exported_symbols_list,${AlususSpp_SOURCE_DIR}/macos_exported_symbols.txt)
elseif(AlususCompilerUsed STREQUAL "GCC" AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(AlususSpp PRIVATE -Wl,--version-script,${AlususSpp_SOURCE_DIR}/linker_version_script.map)
endif()
#TODO: Add export symbols for Windows.

# Copy libary header files to installation directory.
install_files("/${ALUSUS_INCLUDE_DIR_NAME}/Spp" FILES
  spp.h
  LibraryGateway.h
  )
foreach (dir ${AlususSpp_SOURCE_SUBDIRS})
  file(GLOB headers ${dir}/*.h)
  install_files("/${ALUSUS_INCLUDE_DIR_NAME}/Spp/${dir}" FILES ${headers})
endforeach(dir)

install(TARGETS AlususSpp
  RUNTIME DESTINATION ${ALUSUS_BIN_DIR_NAME}
  LIBRARY DESTINATION ${ALUSUS_LIB_DIR_NAME}
  ARCHIVE DESTINATION ${ALUSUS_LIB_DIR_NAME}
)
