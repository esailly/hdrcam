# hdrcam_uvc

## Description

Capture webcam frames with varying exposure times, to compute HDR images, using 
[libuvc](https://github.com/ktossell/libuvc). 

## Dependencies

- g++, cmake, pkg-config
- libusb, jpeg
- opencv 3
- libuvc (clone submodule with `git submodule update --init --recursive`)

## TODO

- compute and display HDR images
- adjust exposure times dynamically
- implement some white balance algorithms
- clean code and fix bugs

