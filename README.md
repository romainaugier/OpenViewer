# OpenViewer
:warning: The project is under development and might not work/compile when you will clone it :warning:

OpenViewer is an open source, tiny and powerful image viewer made for professionals and enthusiasts of the animation and vfx industry.

The wiki (under development) is [here](https://github.com/romainaugier/OpenViewer/wiki), and the roadmap of the project is [here](https://github.com/romainaugier/OpenViewer/wiki/Roadmap).

## Dependencies

OpenViewer uses [OpenImageIO](https://github.com/OpenImageIO/oiio), [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr) and [Wuffs](https://github.com/google/wuffs) for image reading, [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO) to manage colorspaces and [Dear ImGui](https://github.com/ocornut/imgui) for the interface.

## Installation

OpenViewer uses [Vcpkg](https://github.com/microsoft/vcpkg) to manage dependencies and [CMake](https://cmake.org/) to build across platforms. Make sure git and cmake are installed on your machine before building OpenViewer.

You first need to install vcpkg and OpenViewer's dependencies. Depending on your machine capabilities, it can take a non negligeable amount of time to build (up to 1 hour on the small pc I use to develop on linux).

### Linux
```bash
git clone https://github.com/romainaugier/OpenViewer.git
cd OpenViewer
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./vcpkg-bootstrap.sh
./vcpkg install OpenImageIO:x64-linux
./vcpkg install OpenColorIO:x64-linux
```

### Windows
```bash
git clone https://github.com/romainaugier/OpenViewer.git
cd OpenViewer
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
vcpkg-bootstrap.bat
vcpkg install OpenImageIO:x64-windows
vcpkg install OpenColorIO:x64-windows
```


Once all the dependencies are built, you can run the build script. If you run into a *permission denied* on Linux, use the ```chmod +x``` command with the script name.
If you already have vcpkg installed somewhere, you can pass the toolchain file to CMake with -DCMAKE_TOOLCHAIN_FILE="D:/path/to/vcpkg.cmake".


### Linux
```bash
cd Openviewer
cmake -S . -B build
# if you already have vcpkg installed
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="/home/path/to/vcpkg"

cd build
cmake --build . --config Release -j $(numproc)
```

### Windows
```bat
cd OpenViewer
cmake -S . -B build
# if you already have vcpkg installed
# cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="D:/path/to/vcpkg"

cd build
cmake --build . --config Release -j %NUMBER_OF_PROCESSORS%
```

## Copyright

Portions of this software are copyright :

Copyright (c) 2021, Romain Augier

All rights reserved.

## Third Party Softwares :

This software is based in part on the works of :

- [CMake](https://cmake.org/), Copyright © 2000-2021 Kitware, Inc. and Contributors, All rights reserved
- [GLFW](https://www.glfw.org/), Copyright © 2002-2006 Marcus Geelnard, Copyright © 2006-2019 Camilla Löwy
- [Dear ImGui](https://github.com/ocornut/imgui), Copyright © 2014-2021 Omar Cornut
- [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr), Copyright © 2006, Industrial Light and Magic, a division of Lucasfilm Entertainment Company Ltd.
- [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO), Copyright © 2020 OpenColorIO Contributors
- [OpenImageIO](https://github.com/OpenImageIO/oiio), Copyright © 2008-2021 by Contributors to the OpenImageIO project. All Rights Reserved.
- [Wuffs](https://github.com/google/wuffs), Copyright © 2021 Andrew Dassonville, Chris Palmer, Jimmy Casey, Leo Neat, Mike Kaufman, Muhammad Aldo Firmansyah, Nigel Tao
- [Vcpkg](https://github.com/microsoft/vcpkg), Copyright © Microsoft Corporation
