# OpenViewer
OpenViewer is an open source, tiny and powerful image viewer.

## Installation

OpenViewer uses vcpkg to manage dependencies and cmake to build across platforms. Make sure git and cmake are installed on your machine before building OpenViewer.

You first need to install vcpkg and OpenViewer's dependencies. Depending on your machine capabilities, it can take a non negligeable amount of time to build (up to 1 hour on my small pc I use to develop on linux).

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

