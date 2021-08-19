#!/bin/bash
mkdir build
cd build
cmake ..
cmake --build . --config=Release
ctest -C Release
