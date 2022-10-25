

```bat 
conan create . openimageio/2.4@openviewer/1.0 -o with_ffmpeg=False -o boost:shared=True -o openexr:shared=True -o openjpeg:shared=True --build missing 
```

```bat
cd conan/recipes/opencolorio
conan create . opencolorio/2.1.0@openviewer/1.0
```

```bat
conan install . --build missing
```