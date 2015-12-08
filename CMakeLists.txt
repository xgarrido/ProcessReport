# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 2.8)
project(FalaiseProcessReportPlugin)

# Modules use Falaise, so we need to locate this or fail
# find_package(Falaise REQUIRED)

# Ensure our code can see the Falaise headers
#include_directories(${Falaise_INCLUDE_DIRS})
include_directories(${Falaise_BUILDPRODUCT_DIR}/include)
include_directories(${Falaise_BUILDPRODUCT_DIR}/include/falaise)
include_directories(${Bayeux_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)


############################################################################################
# - ProcessReport modules:
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise)

# - Headers:
list(APPEND FalaiseProcessReportPlugin_HEADERS
  source/falaise/snemo/processing/process_report_module.h
  source/falaise/snemo/processing/cut_report_driver.h
  )

# - Sources:
list(APPEND FalaiseProcessReportPlugin_SOURCES
  source/falaise/snemo/processing/process_report_module.cc
  source/falaise/snemo/processing/cut_report_driver.cc
  )

############################################################################################

# # Build a dynamic library from our sources
add_library(Falaise_ProcessReport SHARED
  ${FalaiseProcessReportPlugin_HEADERS}
  ${FalaiseProcessReportPlugin_SOURCES})

target_link_libraries(Falaise_ProcessReport Falaise)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(Falaise_ProcessReport
    PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Install it:
install(TARGETS Falaise_ProcessReport DESTINATION ${CMAKE_INSTALL_LIBDIR}/Falaise/modules)

# Test support:
option(FalaiseProcessReportPlugin_ENABLE_TESTING "Build unit testing system for FalaiseProcessReportPlugin" ON)
if(FalaiseProcessReportPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()
