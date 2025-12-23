# CL Manager
CLManager aims to be an easy to use, single* header file for using OpenCL with C++ on Windows and Linux, with support
for OpenGL interop (only X11 is supported for Linux).

It manages a lot of the tedium of working with OpenCL (creating and managing objects for devices, queues, kernels,
buffers etc.) and provides simple functions for reading and writing buffers, setting kernel ranges and parameters, and
running kernels, with error checking and messages when things go wrong.

The core of the library is the use of maps for storing the various OpenCL objects, with strings as keys. Error messages
will reference the string names of the kernels or buffers which the error relates to, which should make debugging at
least a little bit less painful. The user only needs to store and provide the string identifiers for the objects they
wish to use.

*the code which wraps the OpenCL C++ bindings is in the single header file `CLManager.h`, but code for writing kernels
is split into seperate files (mainly to prevent issues with the `#define`s in `KernelRString.h`). The use of these files
is optional, see the usage section for more details.

## Usage
`#include CLManager.h` anywhere you wish to interact with OpenCL. In **one** .cpp file which includes `CLManager.h`,
`#define CL_MANAGER_IMPL` before the include to create the definitions of the functions.

If writing kernel code as per the example (see the examples section for details) then `kernels.h`, `kernels.cpp` and
`KernelRString.h` are required. `kernels.h` only needs to be included where `CLManager::init(createKernelSource())` is
called, as it will provide the kernel source code which is needed for initialisation.

If using another method for writing kernel code (e.g. separate .cl files), then the kernel code needs to be read into a
string and passed to `CLManager::init()`. The additional files mentioned above are then not required.

`CLManager.h` relies on the OpenCL C++ bindings. The required header and library files can be found here:
https://github.com/KhronosGroup/OpenCL-SDK/releases.

If using Linux, install the relevant packages for your hardware, as they should provide these files. See
https://wiki.archlinux.org/title/General-purpose_computing_on_graphics_processing_units.

The OpenGL interop relies on GLFW: https://www.glfw.org/

## Examples
A basic example showing kernels, buffers and device creation is provided in `main.cpp` in this repository, and defines
the kernels in `kernels.cpp`.

A more comprehensive (and difficult to follow) example is my program for generating
fractals through iterated function systems, which includes working with OpenGL and sharing GPU memory which is
processed with OpenCL and displayed with OpenGL: https://github.com/alexf13e/fractal-flame - mainly look at `ifs.cpp`.

## Notes and Credits
* The method of writing kernel code directly in .cpp files with some degree of syntax highlighting is taken from
ProjectPhysX's OpenCL-Wrapper library, which is worth looking at as an alternative to my library:
https://github.com/ProjectPhysX/OpenCL-Wrapper
* OpenGL interop is enabled by writing `#define CL_MANAGER_GL` in **one** .cpp file; it is best to place it next to the
definition for `CL_MANAGER_IMPL`. The order does not matter, as long as it is before the include.
* Device and context creation is handled automatically when `CLManager::init()` is called. A list of devices on your
computer will be searched until one is found which supports OpenCL and - if requested - memory sharing with OpenGL. If
more control over the device being used is required, modify the `createDevice` functions. Note that there are two
versions of this function; one each for when OpenGL is and is not being used.