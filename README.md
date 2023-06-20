# OpenViewer

The goal of OpenViewer is to provide a lightweight and optimized C++ library to load, modify and display medias along with an application to show how the library can be used. It comes with Python bindings to allow everyone to make its own media viewer.

Disclaimer : it is currently a work in progress, so do not expect everything to be stable and working perfectly. I (Romain Augier) learned to code on my own , so any suggestion on how to improve the code, the design of the library/application or anything else is welcome!

## Installation

To manage its dependencies, OpenViewer uses [Conan 1.57](https://conan.io/).

You can install it in a virtual python environment directly in this directory 
```bash
python -m venv conan_env
source conan_env/bin/activate
pip install conan==1.57
```

You'll need to first build the recipes for OpenColorIO and OpenImageIO (OpenImageIO will soon be removed as it is a huge dependency with too much stuff we do not need).

```bash
source conan_env/bin/activate
cd conan/recipes/opencolorio
conan create . opencolorio/2.1.0@openviewer/1.0
```

```bash
source conan_env/bin/activate
cd conan/recipes/openimageio
conan create . openimageio/2.4@openviewer/1.0 -o with_ffmpeg=False -o boost*:shared=True -o openexr*:shared=True -o openjpeg*:shared=True --build missing
```

If you encounter any error during the recipes building, mentionning the libstdcxx is not the good version, you can update your conan profile :
```bash
source conan_env/bin/activate
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Once you've built the custom recipes, you can install the conanfile that is in the root directory

```bash
source conan_env/bin/activate
conan install . --build missing
```

Then, to build the project, there are utility script : 
- `--debug` turn on debug build
- `--tests` build the tests for all the libraries and run them
- `--clean` remove the previous build folder, if existing
- `--export-compile-commands` export the json file containing compile commands for each file (useful for clangd, not working on windows though)
- `--sanitize` enables instrumentation of the code (in debug only) to sanitize memory addresses and leaks

```bash
source conan_env/bin/activate
./build.sh --debug --tests --clean --export-compile-commands --sanitize
```

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
