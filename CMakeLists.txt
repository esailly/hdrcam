CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( hdrcam )

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" )
endif()

IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
  ADD_DEFINITIONS (-DNDEBUG)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libuvc")
  message(FATAL_ERROR "The libuvc submodule directory is missing! "
    "You probably did not clone the project with --recursive. "
    "It is possible to recover by running:\n"
    "git submodule update --init --recursive")
endif()

INCLUDE_DIRECTORIES ( libuvc/include )

FILE ( GLOB LIBUVC_SOURCE libuvc/src/*.c )

FILE ( GLOB HDRCAM_SOURCE src/*.cpp )

ADD_EXECUTABLE ( hdrcam.out ${LIBUVC_SOURCE} ${HDRCAM_SOURCE} )

INSTALL ( TARGETS hdrcam.out DESTINATION bin )

