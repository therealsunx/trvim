#!/bin/bash


BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
  mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" || {
  echo -e "\x1b[31m-- Failed to navigate to directory: ${BUILD_DIR}\x1b[0m"
  exit 1
}

cmake .. || {
    echo -e "\x1b[31m-- CMake configuration failed. Please check your CMakeLists.txt.\x1b[0m"
    exit 1
}

if [ $? -ne 0 ]; then
    echo -e "\x1b[31m-- CMake configuration failed.\x1b[0m"
    exit 1
fi

echo -e "\x1b[32m-- Build completed successfully.\x1b[0m"

sudo make install || {
  echo -e "\x1b[31m-- Failed to install\x1b[0m"
  exit 1
}

echo -e "\x1b[32m-- Installed successfully.\x1b[0m"
