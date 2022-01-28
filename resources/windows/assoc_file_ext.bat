@echo off

echo Associating file extensions with OpenViewer

set OPENVIEWERPATH = "W:\RESSOURCES\OpenViewer\0.0.2 Alpha Win64\OpenViewer.exe"

rem OpenEXR
echo Associating OpenEXR file extension
assoc .exr=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Hdr
echo Associating Hdr file extension
assoc .hdr=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Png
echo Associating Png file extension
assoc .png=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Tiff
echo Associating Tiff file extension
assoc .tiff=openviewer
assoc .tif=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Jpg
echo Associating Jpg file extension
assoc .jpg=openviewer
assoc .jpeg=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Tex
echo Associating Tex file extension
assoc .tex=openviewer
assoc .ptex=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"

rem Raw
echo Associating Raw file extension
assoc .raw=openviewer
assoc .arw=openviewer
assoc .cr2=openviewer
ftype openviewer=%OPENVIEWERPATH% -p "%1"