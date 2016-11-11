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

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ext/libuvc")
  message(FATAL_ERROR "The libuvc submodule directory is missing! "
    "You probably did not clone the project with --recursive. "
    "It is possible to recover by running:\n"
    "git submodule update --init --recursive")
endif()

find_package( Threads )

find_package( PkgConfig REQUIRED )
pkg_check_modules( MYPKG REQUIRED libusb-1.0 libjpeg opencv )
include_directories( ${MYPKG_INCLUDE_DIRS} )

add_subdirectory( ext/libuvc )
include_directories ( 
    ${CMAKE_BINARY_DIR}/ext/libuvc/include ext/libuvc/include )

set ( LIBUVC_SOURCE 
    ext/libuvc/src/ctrl-gen.c
    ext/libuvc/src/ctrl.c
    ext/libuvc/src/device.c
    ext/libuvc/src/diag.c
    ext/libuvc/src/frame-mjpeg.c
    ext/libuvc/src/frame.c
    ext/libuvc/src/init.c
    ext/libuvc/src/misc.c
    ext/libuvc/src/stream.c
    )

add_executable ( hdrcam.out ${LIBUVC_SOURCE} src/hdrcam.cpp )
target_link_libraries( hdrcam.out 
    ${MYPKG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBUSB_LIBRARIES})

add_executable ( caminfo.out ${LIBUVC_SOURCE} src/caminfo.cpp )
target_link_libraries( caminfo.out 
    ${MYPKG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${LIBUSB_LIBRARIES})

install ( TARGETS hdrcam.out caminfo.out DESTINATION bin )

