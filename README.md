# OpenViewer

The goal of OpenViewer is to provide a lightweight and optimized C++ library to load, modify and display medias along with an application to show how the library can be used. It comes with Python bindings to allow everyone to make its own media viewer.

Disclaimer : it is currently a work in progress, so do not expect everything to be stable and working perfectly. I (Romain Augier) learned to code on my own , so any suggestion on how to improve the code, the design of the library/application or anything else is welcome!

## Platforms

| OS | Architecture | Compiler | CI |
| --- | --- | --- | --- |
| Linux | x86_64, aarch64 | gcc 11+, clang 14+ | yes |
| macOS 12+ | arm64 (Apple silicon), x86_64 | AppleClang 14+ | arm64 |
| Windows 10+ | x86_64 | MSVC 2022 | yes |

32 bits targets are not supported.

## Building

Dependencies come from [vcpkg](https://github.com/microsoft/vcpkg) in manifest
mode (`vcpkg.json`), except [stdromano](https://github.com/romainaugier/stdromano),
which is a submodule built first and installed into `ext/stdromano/install`.
stdromano also provides spdlog: do not install another one.

System packages:

- Linux: `cmake`, `ninja`, `pkg-config`, `nasm` (x86_64 only), `libgtk2.0-dev` (optional, file dialogs)
- macOS: Xcode command line tools, then `brew install cmake ninja pkg-config`
- Windows: Visual Studio 2022 with the C++ workload

```bash
git clone --recursive https://github.com/romainaugier/OpenViewer.git
cd OpenViewer

# 1. stdromano, installed where OpenViewer looks for it
cd ext/stdromano && ./build.sh --install --installdir:$PWD/install && cd ../..

# 2. OpenViewer, with the tests
./build.sh --tests
```

On Windows use `build.bat` with the same arguments.

`build.sh` options: `--debug`, `--reldebug`, `--tests`, `--clean`, `--install`,
`--installdir:<path>`, `--vcpkgpath:<path>`, `--addrsan`, `--ubsan`, `--leaksan`,
`--threadsan`, `--export-compile-commands`.

To build x86_64 binaries on an Apple silicon mac, configure with
`-DCMAKE_OSX_ARCHITECTURES=x86_64`; the architecture specific flags follow it.

## Acknowledgement

OpenViewer is based on the work of many good libraries : 
- [FMT](https://github.com/fmtlib/fmt)
- [Spdlog](https://github.com/gabime/spdlog)
- [OpenColorIO](https://opencolorio.org)
- [OpenImageIO](https://sites.google.com/site/openimageio/home)
- [OpenEXR](https://openexr.com)
- [OpenGL](https://www.opengl.org)
- [GLFW](https://www.glfw.org)
- [Glew](https://github.com/nigels-com/glew)
- [OpenCL](https://www.khronos.org/opencl)
- [STB](https://github.com/nothings/stb)
- [Nlohmann-json](https://github.com/nlohmann/json)
- [Pybind11](https://github.com/pybind/pybind11)
- [TSL](https://github.com/Tessil/robin-map)
- [ImGui](https://github.com/ocornut/imgui)
