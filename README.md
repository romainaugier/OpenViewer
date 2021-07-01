# OpenViewer
OpenViewer is an open source, tiny and powerful image viewer made for professionals and enthusiasts of the animation and vfx industry.

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
./vcpkg install OpenColorIO
./vcpkg install OpenImageIO
```

If you have an issue while building OpenColorIO with vcpkg, look at the log but it might be possible that it requires libraries from the system that are not installed. To solve this issue, open a terminal and run the following command:
```bash
sudo apt-get install libxi-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev libxrandr-dev libxxf86vm-dev
```

### Windows
```bash
git clone https://github.com/romainaugier/OpenViewer.git
cd OpenViewer
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./vcpkg-bootstrap.bat
./vcpkg install OpenColorIO
./vcpkg install OpenImageIO
```


Once all the dependencies are built, you can run the build script. If you run into a *permission denied* on Linux, use the ```chmod +x``` command with the script name.

### Linux
```bash
cd Openviewer
./build_linux.sh
```

### Windows
```bash
cd OpenViewer
./build_windows
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
