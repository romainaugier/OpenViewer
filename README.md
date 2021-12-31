# OpenViewer
![build windows](https://github.com/romainaugier/OpenViewer/actions/workflows/build-windows.yml/badge.svg)
![build linux](https://github.com/romainaugier/OpenViewer/actions/workflows/build-linux.yml/badge.svg)

OpenViewer is an open source, tiny and powerful image viewer made for professionals and enthusiasts of the animation and vfx industry.

As I am currently in the last year of my studies and have a lot of work to be done for school, I don't have a lot of time to work on the project, but I'll do my best to advance it from time to time.

OpenViewer is a personal project and initiative since, after speaking with a few professionals from the industry, there is a lack in a good an well maintained image player. I've decided to start the development of OpenViewer to learn more about software engineering and development, color management, image formats, C++... As I am a self-learner, and I've started coding seriously in 2020, be aware that there might be some awkward errors, bugs and not well-designed/optimized code, and so you're welcome to give any feedback by reporting an issue in the github repository, or you can email me at contact@romainaugier.com for more in-depth reviews and mention OpenViewer in the email object.

You can sponsor the project by clicking on the sponsor button, and it will help me unlock more time to work on the project.

The wiki (under development) is [here](https://github.com/romainaugier/OpenViewer/wiki), and the roadmap of the project is [here](https://github.com/romainaugier/OpenViewer/wiki/Roadmap).

If you or your studio are interested in implementing special features/need help to build the software and integrate it in your pipeline, you can email me at contact@romainaugier.com so we can discuss about it.

## Dependencies

OpenViewer uses [OpenImageIO](https://github.com/OpenImageIO/oiio), [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr) and [Wuffs](https://github.com/google/wuffs) for image reading, [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO) to manage colorspaces, [Dear ImGui](https://github.com/ocornut/imgui) for the interface and [Glew](https://github.com/nigels-com/glew) to load OpenGL functions.

## Installation

OpenViewer uses [Vcpkg](https://github.com/microsoft/vcpkg) to manage dependencies and [CMake](https://cmake.org/) to build across platforms. Make sure Git and CMake are installed on your machine before building OpenViewer.

Vcpkg commit ID used to built the dependencies : 1085a57da0725c19e19586025438e8c16f34c890. 
If you want to build the project and match the dependencies versions I use to develop OpenViewer and used in Github Actions, rebase vcpkg to this commit.

You first need to install vcpkg and OpenViewer's dependencies. Depending on your machine capabilities, it can take a non negligeable amount of time to build (up to 1 hour on the small pc I use to develop on linux).

### Linux
```bash
git clone https://github.com/romainaugier/OpenViewer.git
cd OpenViewer
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./vcpkg-bootstrap.sh
./vcpkg install openimageio:x64-linux
./vcpkg install opencolorio:x64-linux
./vcpkg install glew:x64-linux
```

### Windows
```bash
git clone https://github.com/romainaugier/OpenViewer.git
cd OpenViewer
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
vcpkg-bootstrap.bat
vcpkg install openimageio:x64-windows
vcpkg install opencolorio:x64-windows
vcpkg install glew:x64-windows
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
rem if you already have vcpkg installed
rem cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="D:/path/to/vcpkg"

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

## Special thanks

Thanks to Paul-Emile Buteau for the help and the code reviews.

Thanks to Legend for his 20k jpg renders that helps testing the application.
