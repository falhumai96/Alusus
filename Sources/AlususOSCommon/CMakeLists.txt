project(AlususOSCommon)
cmake_minimum_required(VERSION 3.20.0)

set(AlususOSCommon_Source_Files AlususOSCommon.cpp AlususOSCommon.hpp)
add_library(AlususOSCommon STATIC ${AlususOSCommon_Source_Files})
target_include_directories(AlususOSCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (ALUSUS_WIN32_UNICODE)
  target_compile_definitions(AlususOSCommon PRIVATE UNICODE ALUSUS_WIN32_UNICODE)
endif()
