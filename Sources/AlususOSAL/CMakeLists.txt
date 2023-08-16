project(AlususOSAL)
cmake_minimum_required(VERSION 3.20.0)

set(AlususOSAL_Source_Files AlususOSAL.cpp AlususOSAL.hpp)
add_library(AlususOSAL STATIC ${AlususOSAL_Source_Files})
target_include_directories(AlususOSAL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususOSAL PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()
set_target_properties(AlususOSAL PROPERTIES
  COMPILE_FLAGS "${Alusus_COMPILE_FLAGS}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  C_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  OUTPUT_NAME alusus_osal
  DEBUG_OUTPUT_NAME alusus_osal
  VERSION ${AlususVersion}
)
