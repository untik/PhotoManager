@echo off

set vcpkg_dir=vcpkg_installed\x64-windows-v142
set projectdir=%~dp0

rem Copy DLLs to bin64 and bin64_debug

echo \bin64
if not exist %projectdir%bin64 mkdir %projectdir%bin64

echo \bin64\exiv2.dll
copy /Y %vcpkg_dir%\bin\exiv2.dll %projectdir%bin64\exiv2.dll

echo \bin64\jpeg62.dll
copy /Y %vcpkg_dir%\bin\jpeg62.dll %projectdir%bin64\jpeg62.dll

echo \bin64\libexpat.dll
copy /Y %vcpkg_dir%\bin\libexpat.dll %projectdir%bin64\libexpat.dll

echo \bin64\liblzma.dll
copy /Y %vcpkg_dir%\bin\liblzma.dll %projectdir%bin64\liblzma.dll

echo \bin64\tiff.dll
copy /Y %vcpkg_dir%\bin\tiff.dll %projectdir%bin64\tiff.dll

rem echo \bin64\turbojpeg.dll
rem copy /Y %vcpkg_dir%\bin\turbojpeg.dll %projectdir%bin64\turbojpeg.dll

echo \bin64\zlib1.dll
copy /Y %vcpkg_dir%\bin\zlib1.dll %projectdir%bin64\zlib1.dll



echo \bin64_debug
if not exist %projectdir%bin64_debug mkdir %projectdir%bin64_debug

echo \bin64_debug\exiv2.dll
copy /Y %vcpkg_dir%\debug\bin\exiv2.dll %projectdir%bin64_debug\exiv2.dll

echo \bin64_debug\jpeg62.dll
copy /Y %vcpkg_dir%\debug\bin\jpeg62.dll %projectdir%bin64_debug\jpeg62.dll

echo \bin64_debug\libexpatd.dll
copy /Y %vcpkg_dir%\debug\bin\libexpatd.dll %projectdir%bin64_debug\libexpatd.dll

echo \bin64_debug\liblzma.dll
copy /Y %vcpkg_dir%\debug\bin\liblzma.dll %projectdir%bin64_debug\liblzma.dll

echo \bin64_debug\tiffd.dll
copy /Y %vcpkg_dir%\debug\bin\tiffd.dll %projectdir%bin64_debug\tiffd.dll

rem echo \bin64_debug\turbojpeg.dll
rem copy /Y %vcpkg_dir%\debug\bin\turbojpeg.dll %projectdir%bin64_debug\turbojpeg.dll

echo \bin64_debug\zlibd1.dll
copy /Y %vcpkg_dir%\debug\bin\zlibd1.dll %projectdir%bin64_debug\zlibd1.dll
