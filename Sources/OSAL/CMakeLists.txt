project(AlususOSAL)
cmake_minimum_required(VERSION 3.20.0)

if(NOT ALUSUS_OSAL_APR_MANUAL_CONFIG) # Try to auto-detect APR package configuration using pkg-config.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ALUSUS_OSAL_APR_PKGCONFIG REQUIRED apr-1)
endif()

if(WIN32)
  set(AlususOSAL_Source_Files ${AlususOSAL_Source_Files} "${CMAKE_CURRENT_SOURCE_DIR}/Win32/OSAL.cpp")
else()
  set(AlususOSAL_Source_Files ${AlususOSAL_Source_Files} "${CMAKE_CURRENT_SOURCE_DIR}/Unix/OSAL.cpp")

  if(APPLE)
    set(AlususOSAL_Source_Files ${AlususOSAL_Source_Files} "${CMAKE_CURRENT_SOURCE_DIR}/Unix/Apple/OSAL.cpp")
  else()
    set(AlususOSAL_Source_Files ${AlususOSAL_Source_Files} "${CMAKE_CURRENT_SOURCE_DIR}/Unix/Linux/OSAL.cpp")
  endif()
endif()

set(AlususOSAL_Source_Files ${AlususOSAL_Source_Files} "${CMAKE_CURRENT_SOURCE_DIR}/Common/OSAL.cpp")

message("AlususOSAL_Source_Files=${AlususOSAL_Source_Files}")

add_library(AlususOSAL STATIC ${AlususOSAL_Source_Files})

target_include_directories(AlususOSAL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")

set_target_properties(AlususOSAL PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
)

# For APR integration with OSAL.
if(ALUSUS_OSAL_APR_MANUAL_CONFIG)
  if(ALUSUS_OSAL_APR_MANUAL_CONFIG_INCLUDE_DIR)
    target_include_directories(AlususOSAL PUBLIC "${ALUSUS_OSAL_APR_MANUAL_CONFIG_INCLUDE_DIR}")
  endif()

  if(ALUSUS_OSAL_APR_MANUAL_CONFIG_DEFS)
    target_compile_definitions(AlususOSAL PUBLIC "${ALUSUS_OSAL_APR_MANUAL_CONFIG_DEFS}")
  endif()

  if(ALUSUS_OSAL_APR_MANUAL_CONFIG_LIB_DIR)
    find_library(APRLib apr-1 HINTS ${ALUSUS_OSAL_APR_MANUAL_CONFIG_LIB_DIR})
  else()
    find_library(APRLib apr-1)
  endif()

  target_link_libraries(AlususOSAL PUBLIC ${APRLib})
  unset(${APRLib})

  if(ALUSUS_OSAL_APR_MANUAL_CONFIG_LIBS)
    target_link_libraries(AlususOSAL PUBLIC "${ALUSUS_OSAL_APR_MANUAL_CONFIG_LIBS}")
  endif()
else()
  # It should contain the include directory and compile definitions.
  if(ALUSUS_OSAL_APR_PKGCONFIG_CFLAGS)
    target_compile_options(AlususOSAL PUBLIC "${ALUSUS_OSAL_APR_PKGCONFIG_CFLAGS}")
  endif()

  # It should contain the library directory and the link libraries.
  if(ALUSUS_OSAL_APR_PKGCONFIG_LDFLAGS)
    target_link_libraries(AlususOSAL PUBLIC "${ALUSUS_OSAL_APR_PKGCONFIG_LDFLAGS}")
  endif()
endif()

# For Cwalk integration with OSAL.
find_package(Cwalk CONFIG REQUIRED)
target_link_libraries(AlususOSAL PUBLIC cwalk)
