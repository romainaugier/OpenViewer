conan 1.56

sudo pacman -S libxvmc

Fixes for opencolorio : pystring include directory missing, and <cstring> missing in utils/StringUtils.h

```bat
cd conan/recipes/opencolorio
conan create . opencolorio/2.1.0@openviewer/1.0
```

```bat 
cd conan/recipes/openimageio
conan create . openimageio/2.4@openviewer/1.0 -o with_ffmpeg=False -o boost*:shared=True -o openexr*:shared=True -o openjpeg*:shared=True --build missing
```

```bat
conan install . --build missing
```