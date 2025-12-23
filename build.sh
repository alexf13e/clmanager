#!/usr/bin/bash

output=build/clmanager_example
sources="./*.cpp"
includes="-I ../_headers"
libs="-lOpenCL"

g++ -Wall -Wextra -o $output $sources $includes $libs
