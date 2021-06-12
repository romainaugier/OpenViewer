# OpenViewer
OpenViewer is an open source, tiny and powerful image viewer made for professionals and enthusiasts of the animation and vfx industry.

## Dependencies

OpenViewer uses [OpenImageIO](https://github.com/OpenImageIO/oiio) for image reading, [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO) to manage colorspaces and [Dear ImGui](https://github.com/ocornut/imgui) for the interface.

---
## Installation

OpenViewer uses vcpkg to manage dependencies and cmake to build across platforms. Make sure git and cmake are installed on your machine before building OpenViewer.

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

---
## Roadmap

- File types supported :
  - :heavy_check_mark: OpenEXR
    - :heavy_check_mark: fast loading (approx 10ms from SATA on ryzen 7 2700x) 
    - :x: multi-part loading
  - :heavy_check_mark: PNG
    - :x: fast loading
    - :x: alpha as a separate channel
  - :heavy_check_mark: JPG
    - :x: fast loading
  - :heavy_check_mark: Other file formats
    - :heavy_check_mark: TIFF, JPEG/JFIF, HDR/RGBE, ICO, BMP, Targa, JPEG-2000, RMan Zfile, FITS, DDS, Softimage PIC, PNM, DPX, Cineon, IFF, Field3D, OpenVDB, Ptex, Photoshop PSD, Wavefront RLA, SGI, WebP, GIF, DICOM, HEIF/HEIC/AVIF, many "RAW" digital camera formats
    - :x: MP4, MOV

- System :
  - :heavy_check_mark: File loading
  - :clock9: File cache
  - :heavy_check_mark: Asynchronous loading
  - :x: Command line interface
    - :heavy_check_mark: Single image
    - :heavy_check_mark: Folder
    - :x: Python script
  - :heavy_check_mark: Open file within the application
  - :x: Persistent user settings (saved as a .json file)
  - :x: Sound playing

- Display :
  - :clock9: OCIO Display Transform
  - :x: Exposure modification
  - :x: Multiple displays
  - :x: OpenEXR multi layer display
  - :x: OpenEXR multi layer contact sheet
  - :x: Annotations : 
    - :x: Text annotations on the image
    - :x: Drawing annotations
    - :x: Onion peel through frame sequence annotations

- Plot :
  - :x: Histogram
  - :x: Waveform
  - :x: Vector Scope
  - :x: Custom plot

- UI :
  - :x: Playbar improvement (it's really ugly for now)
  - :x: Display improvement (zoom and drag)
  - :x: Settings windows

- Python :
  - :x: API Bindings
  - :x: Console 
  - :x: Multi line scripts execution