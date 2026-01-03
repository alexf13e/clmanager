output=./build/mandelbrot
sources="./*.cpp ../../../../cpp/_src/glad.c"
includes="-I ../../../../cpp/_headers"
libs="-lOpenCL -lglfw -lGL -lpthread -ldl -lX11 -lXrandr -lXi"

g++ -Wall -Wextra -o $output $sources $includes $libs