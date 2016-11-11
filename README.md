# HDRCAM

## Description

Capture webcam frames with varying exposure times, to compute HDR images.

## Dependencies

- build tools: g++, cmake, pkg-config
- webcam capture: libusb, libjpeg
- image processing: opencv 3

## Building

```
git clone https://github.com/juliendehos/hdrcam --recursive
mkdir hdrcam/build
cd hdrcam/build
cmake ..
make
sudo ./hdrcam.out  # or adjust udev rules to execute as a normal user
```

This should clone and build the [https://github.com/ktossell/libuvc](libuvc)
submodule automatically.

## TODO

- compute and display HDR images
- adjust exposure times dynamically
- implement some white balance algorithms
- clean code and fix bugs

