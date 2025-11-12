cmake_minimum_required(VERSION 3.19)
project(targ LANGUAGES CXX)

# Library settings
set(TARG_VERSION 0.1.0)
set(TARG_INCLUDE_DIR include)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_EXAMPLES "Build example programs" ON)
option(INSTALL_HEADERS_TO_SYSTEM_DIR "Install headers to system include dir (/usr/local/include or CMAKE_INSTALL_PREFIX/include)" ON)

# Public header-only target
add_library(targ INTERFACE)
target_include_directories(targ
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${TARG_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>         # used when installed
)
target_compile_features(targ INTERFACE cxx_std_23)

# Examples
if(BUILD_EXAMPLES)
  file(GLOB EXAMPLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/examples/*.cpp)
  foreach(example ${EXAMPLE_SRCS})
    get_filename_component(name ${example} NAME_WE)
    add_executable(${name} ${example})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    #target_link_libraries(${name} PRIVATE targ)
    # enable warnings for examples
    target_compile_options(${name} PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
      $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
      $<$<CXX_COMPILER_ID:MSVC>:/W3>
    )
  endforeach()
endif()

include(GNUInstallDirs)

file(GLOB_RECURSE TARG_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${TARG_INCLUDE_DIR}/*.hpp)

if(INSTALL_HEADERS_TO_SYSTEM_DIR)
  install(FILES ${TARG_HEADERS}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/targ
          COMPONENT dev)
else()
  install(DIRECTORY ${TARG_INCLUDE_DIR}/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/targ
          FILES_MATCHING PATTERN "*.hpp"
          COMPONENT dev)
endif()

install(TARGETS targ
  EXPORT targTargets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/targ
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/targConfigVersion.cmake"
  VERSION ${TARG_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/targConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/targ
)

install(EXPORT targTargets
  FILE targTargets.cmake
  NAMESPACE targ::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/targ
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/targConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/targConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/targ
)

set(CPACK_PACKAGE_NAME "targ")
set(CPACK_PACKAGE_VENDOR "ngg")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Header-only minimal command-line arg parsing (ngg::targ)")
set(CPACK_PACKAGE_VERSION ${TARG_VERSION})
include(CPack)
