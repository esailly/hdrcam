CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( hdrcam_v4l )

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" )
endif()

IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
  ADD_DEFINITIONS (-DNDEBUG)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

find_package( Threads )

find_package( PkgConfig REQUIRED )
pkg_check_modules( MYPKG REQUIRED opencv )
include_directories( ${MYPKG_INCLUDE_DIRS} )

add_executable ( hdrcam_v4l.out src/hdrcam_v4l.cpp )
target_link_libraries( hdrcam_v4l.out 
    ${MYPKG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

install ( TARGETS hdrcam_v4l.out DESTINATION bin )

