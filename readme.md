# CL Manager
CLManager aims to be an easy to use, single* header wrapper for using OpenCL with C++ on Windows and Linux, with support
for OpenGL interop (only X11 is supported for Linux).

It manages a lot of the tedium of working with OpenCL (creating and managing objects for devices, command queue,
kernels, buffers etc.) and provides simple functions for reading and writing buffers, setting kernel ranges and
parameters, and running kernels, with error checking and messages when things go wrong.

The core of the library is the use of maps for storing the various OpenCL objects, with strings as keys. Error messages
will reference the string names of the kernels or buffers which the error relates to, which should make debugging at
least a little bit less painful. The user only needs to store and provide the string identifiers for the objects they
wish to use.

*the code which wraps the OpenCL C++ bindings is in the single header file `CLManager.h`, but code for writing kernels
is split into seperate files (mainly to prevent issues with the `#define`s in `KernelRString.h`). The use of these files
is optional, see the usage section for more details.

## Usage
`CLManager.h` relies on the OpenCL C++ bindings. The required header and library files can be found here:
https://github.com/KhronosGroup/OpenCL-SDK/releases.

If using Linux, install the relevant packages for your hardware, as they should provide these files. See
https://wiki.archlinux.org/title/General-purpose_computing_on_graphics_processing_units.

The OpenGL interop relies on GLFW: https://www.glfw.org/

`#include CLManager.h` anywhere you wish to interact with OpenCL. In **one** .cpp file which includes `CLManager.h`,
`#define CL_MANAGER_IMPL` before the include to create the definitions of the functions.

If writing kernel code as per the examples (see the examples section for details) then `kernels.h`, `kernels.cpp` and
`KernelRString.h` are required. `kernels.h` only needs to be included where `CLManager::init(createKernelSource())` is
called, as it will provide the kernel source code which is needed for initialisation.

If using another method for writing kernel code (e.g. separate .cl files), then the kernel code needs to be read into a
string and passed to `CLManager::init()`. `kernels.h`, `kernels.cpp` and `KernelRString.h` are then not required.

## Examples
There are two example projects in the examples folder.

### Basic Example
The "basic" example shows initialisation of OpenCL, creation of buffers and kernels, running the kernels and reading
back values from buffers. `main.cpp` and `kernels.cpp` respectively contain the code for setting up running kernels,
and the kernel code itself.

### Mandelbrot
The Mandelbrot example shows how a buffer can be shared between OpenCL and OpenGL. The kernel calculates sample values,
which are written to a buffer. This buffer is then read by a fragment shader to produce colours for each pixel in the
window. The view can be moved around with `WASD` and zoom with `Q` and `E`. The maximum number of iterations (i.e.
detail level) can be changed with `R` and `F`, and supersampling can be changed with `T` and `G`.

Building this example requires editing `build.sh` to tell it where relevent files are for GLFW and Glad.

It is of course much simpler to make a program for visualising the Mandelbrot set with just a fragment shader, but this
application was chosen as a (hopefully) easy to follow minimal example of sharing data between OpenCL and OpenGL.

### Iterated Function System Fractals
A larger (and more difficult to follow) example is my program for generating fractals through iterated function systems,
which includes working with sharing memory with OpenGL: https://github.com/alexf13e/fractal-flame - mainly look at
`ifs.cpp`.

It also has a Visual Studio project attached which should allow for easier building on Windows. Make sure to edit the
include and library directories to point wherever you have GLFW/Glad/OpenCL installed. There are also other requirements
stated in the readme.

## Notes and Credits
* OpenGL interop is enabled by writing `#define CL_MANAGER_GL` in **one** .cpp file; it is best to place it next to the
definition for `CL_MANAGER_IMPL`. The order of the definitions does not matter, as long as they are before the include.
* Device and context creation is handled automatically when `CLManager::init()` is called. A list of devices on your
computer will be searched until one is found which supports OpenCL and - if requested - memory sharing with OpenGL. If
more control over the device being used is required, modify the `createDevice` functions. Note that there are two
versions of this function; one each for when OpenGL is and is not being used.
* An alternative good helper library for OpenCL can be found here, which inspired the creation of mine: https://github.com/ProjectPhysX/OpenCL-Wrapper
