# Copyright (C) 2022 Sarmad Khalid Abdullah
#
# This file is released under Alusus Public License, Version 1.0.
# For details on usage and copying conditions read the full license in the
# accompanying license file or at <https://alusus.org/license.html>.

# NOTE: For alusus_storage, we'll use the same filename for both debug and
#       release versions. This is important in cases where the user is running
#       the debug version of the core and is including a release version of a
#       library that also need to link to alusus_storage. We should only have
#       one alusus_storage loaded so that there is only one global storage.
#       Having two separate global storages may result in undefined behavior.

project(AlususCore)
cmake_minimum_required(VERSION 3.20.0)

# Header and library files paths for AlususSrl
include_directories("${AlususSrl_SOURCE_DIR}")

# Make sure the compiler finds the source files.
include_directories("${AlususCore_SOURCE_DIR}")
set(CMAKE_MODULE_PATH "${AlususCore_SOURCE_DIR}")

# Add a target for Alusus global storage.
set(AlususStorage_Source_Files core_global_storage.cpp core_global_storage.h)
add_library(AlususStorage SHARED ${AlususStorage_Source_Files})
set_target_properties(AlususStorage PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  C_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  OUTPUT_NAME alusus_storage
  DEBUG_OUTPUT_NAME alusus_storage
  VERSION ${AlususVersion}
  SHLIBVERSION ${AlususShlibVersion}
)
if (ALUSUS_USE_RPATH)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(AlususStorage PROPERTIES
      INSTALL_RPATH "@executable_path;@executable_path/../${ALUSUS_LIB_DIR_NAME}/"
    )
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(AlususStorage PROPERTIES
      INSTALL_RPATH "$ORIGIN;$ORIGIN/../${ALUSUS_LIB_DIR_NAME}/"
    )
  endif()
endif()
target_link_libraries(AlususStorage PRIVATE AlususOSAL ${CMAKE_DL_LIBS} nowide::nowide)
target_include_directories(AlususStorage PRIVATE "${CMAKE_BINARY_DIR}/AlususDefs")
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususStorage PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()

# Add a target for Alusus core library.
set(AlususCoreLib_Source_Subdirs Basic Data Data/Grammar Data/Ast Notices Processing Processing/Handlers Main)
set(AlususCoreLib_Source_Files core.h)
foreach (DIR ${AlususCoreLib_Source_Subdirs})
  file(GLOB headers ${DIR}/*.h)
  file(GLOB sources ${DIR}/*.cpp)
  source_group("HeaderFiles\\${DIR}" FILES ${headers})
  source_group("SourceFiles\\${DIR}" FILES ${sources})
  set(AlususCoreLib_Source_Files ${AlususCoreLib_Source_Files} ${sources} ${headers})
endforeach(DIR)
add_library(AlususCoreLib STATIC ${AlususCoreLib_Source_Files})
set_target_properties(AlususCoreLib PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  C_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  OUTPUT_NAME alusus_core
  DEBUG_OUTPUT_NAME alusus_core.dbg
  VERSION ${AlususVersion}
)
target_precompile_headers(AlususCoreLib PRIVATE "core.h")
target_link_libraries(AlususCoreLib PRIVATE AlususOSAL ${CMAKE_DL_LIBS} nowide::nowide)
target_include_directories(AlususCoreLib PRIVATE "${CMAKE_BINARY_DIR}/AlususDefs")
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususCoreLib PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()

# Add a target for Alusus core executable.
set(AlususCore_Source_Files start.cpp interop.cpp)
add_executable(AlususCore ${AlususCore_Source_Files})
set_target_properties(AlususCore PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  C_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  OUTPUT_NAME alusus
  DEBUG_OUTPUT_NAME alusus.dbg
  VERSION ${AlususVersion}
)
target_precompile_headers(AlususCore PRIVATE "core.h")
target_include_directories(AlususCore PRIVATE "${CMAKE_BINARY_DIR}/AlususDefs")
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususCore PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()

# Finally, we link the executable to the libraries.
target_link_libraries(AlususCore PRIVATE AlususSrlLib AlususCoreLib AlususStorage AlususOSAL ${CMAKE_DL_LIBS} nowide::nowide)

if (ALUSUS_USE_RPATH)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(AlususCore PROPERTIES
      INSTALL_RPATH "@executable_path;@executable_path/../${ALUSUS_LIB_DIR_NAME}/"
    )
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(AlususCore PROPERTIES
      INSTALL_RPATH "$ORIGIN;$ORIGIN/../${ALUSUS_LIB_DIR_NAME}/"
    )
  endif()
endif()

# Copy libary header files to installation directory.
install_files("/${ALUSUS_INCLUDE_DIR_NAME}/Core" FILES core.h core_global_storage.h)
foreach (DIR ${AlususCoreLib_Source_Subdirs})
  file(GLOB headers ${DIR}/*.h)
  install_files("/${ALUSUS_INCLUDE_DIR_NAME}/Core/${DIR}" FILES ${headers})
endforeach(DIR)

# Install library and executable files.
install(TARGETS AlususStorage AlususCoreLib AlususCore
  RUNTIME DESTINATION ${ALUSUS_BIN_DIR_NAME}
  LIBRARY DESTINATION ${ALUSUS_LIB_DIR_NAME}
  ARCHIVE DESTINATION ${ALUSUS_LIB_DIR_NAME}
)

# Add an install command to create the Arabic symbolic link.
if ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
  set(AlususArabicLink ${CMAKE_INSTALL_PREFIX}/${ALUSUS_BIN_DIR_NAME}/الأسس.نقح)
  set(AlususArabicLinkTarget alusus.dbg)
else()
  set(AlususArabicLink ${CMAKE_INSTALL_PREFIX}/${ALUSUS_BIN_DIR_NAME}/الأسس)
  set(AlususArabicLinkTarget alusus)
endif()
string(REPLACE "//" "/" AlususArabicLink ${AlususArabicLink})
string(REPLACE "//" "/" AlususArabicLinkTarget ${AlususArabicLinkTarget})

install(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    ${AlususArabicLinkTarget} \
    ${AlususArabicLink}   \
    COMMAND_ERROR_IS_FATAL ANY)"
)
