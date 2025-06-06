@echo off

set qtdir=c:\Qt\5.15.2\msvc2019_64
set projectdir=%~dp0

set vcredistdir="c:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Redist\MSVC\14.29.30133\x64\Microsoft.VC142.CRT\"
set vcredistdird="c:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Redist\MSVC\14.29.30133\debug_nonredist\x64\Microsoft.VC142.DebugCRT\"
set ucrtdir="c:\Program Files (x86)\Windows Kits\10\Redist\10.0.18362.0\ucrt\DLLs\x64\"
set ucrtdird="c:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\ucrt\"


rem Create bin, bin_debug directories
echo.
echo \bin64
if not exist %projectdir%bin64 mkdir %projectdir%bin64

echo \bin64\platforms
if not exist %projectdir%bin64\platforms mkdir %projectdir%bin64\platforms

echo \bin64\imageformats
if not exist %projectdir%bin64\imageformats mkdir %projectdir%bin64\imageformats



echo \bin64_debug
if not exist %projectdir%bin64_debug mkdir %projectdir%bin64_debug

echo \bin64_debug\platforms
if not exist %projectdir%bin64_debug\platforms mkdir %projectdir%bin64_debug\platforms

echo \bin64_debug\imageformats
if not exist %projectdir%bin64_debug\imageformats mkdir %projectdir%bin64_debug\imageformats



echo \bin64\Qt5Core.dll
copy /Y %qtdir%\bin\Qt5Core.dll %projectdir%bin64\Qt5Core.dll

echo \bin64\Qt5Concurrent.dll
copy /Y %qtdir%\bin\Qt5Concurrent.dll %projectdir%bin64\Qt5Concurrent.dll

echo \bin64\Qt5Gui.dll
copy /Y %qtdir%\bin\Qt5Gui.dll %projectdir%bin64\Qt5Gui.dll

echo \bin64\Qt5Gui.dll
copy /Y %qtdir%\bin\Qt5Gui.dll %projectdir%bin64\Qt5Gui.dll

echo \bin64\Qt5Widgets.dll
copy /Y %qtdir%\bin\Qt5Widgets.dll %projectdir%bin64\Qt5Widgets.dll

echo \bin64\Qt5Svg.dll
copy /Y %qtdir%\bin\Qt5Svg.dll %projectdir%bin64\Qt5Svg.dll

echo \bin64\platforms\qwindows.dll
copy /Y %qtdir%\plugins\platforms\qwindows.dll %projectdir%bin64\platforms\qwindows.dll

echo \bin64\imageformats\qgif.dll
copy /Y %qtdir%\plugins\imageformats\qgif.dll %projectdir%bin64\imageformats\qgif.dll

echo \bin64\imageformats\qjpeg.dll
copy /Y %qtdir%\plugins\imageformats\qjpeg.dll %projectdir%bin64\imageformats\qjpeg.dll

echo \bin64\imageformats\qtga.dll
copy /Y %qtdir%\plugins\imageformats\qtga.dll %projectdir%bin64\imageformats\qtga.dll

echo \bin64\imageformats\qtiff.dll
copy /Y %qtdir%\plugins\imageformats\qtiff.dll %projectdir%bin64\imageformats\qtiff.dll

echo \bin64\imageformats\qwebp.dll
copy /Y %qtdir%\plugins\imageformats\qwebp.dll %projectdir%bin64\imageformats\qwebp.dll

echo \bin64\imageformats\qico.dll
copy /Y %qtdir%\plugins\imageformats\qico.dll %projectdir%bin64\imageformats\qico.dll

echo \bin64\imageformats\qsvg.dll
copy /Y %qtdir%\plugins\imageformats\qsvg.dll %projectdir%bin64\imageformats\qsvg.dll

echo \bin64\msvcp140.dll
copy /Y %vcredistdir%\msvcp140.dll %projectdir%bin64\msvcp140.dll

echo \bin64\msvcp140_1.dll
copy /Y %vcredistdir%\msvcp140_1.dll %projectdir%bin64\msvcp140_1.dll

echo \bin64\vcruntime140.dll
copy /Y %vcredistdir%\vcruntime140.dll %projectdir%bin64\vcruntime140.dll

echo \bin64\ucrtbase.dll
copy /Y %ucrtdir%\ucrtbase.dll %projectdir%bin64\ucrtbase.dll



echo \bin64_debug\Qt5Cored.dll
copy /Y %qtdir%\bin\Qt5Cored.dll %projectdir%bin64_debug\Qt5Cored.dll

echo \bin64_debug\Qt5Concurrentd.dll
copy /Y %qtdir%\bin\Qt5Concurrentd.dll %projectdir%bin64_debug\Qt5Concurrentd.dll

echo \bin64_debug\Qt5Guid.dll
copy /Y %qtdir%\bin\Qt5Guid.dll %projectdir%bin64_debug\Qt5Guid.dll

echo \bin64_debug\Qt5Guid.dll
copy /Y %qtdir%\bin\Qt5Guid.dll %projectdir%bin64_debug\Qt5Guid.dll

echo \bin64_debug\Qt5Widgetsd.dll
copy /Y %qtdir%\bin\Qt5Widgetsd.dll %projectdir%bin64_debug\Qt5Widgetsd.dll

echo \bibin64_debugn64\Qt5Svgd.dll
copy /Y %qtdir%\bin\Qt5Svgd.dll %projectdir%bin64_debug\Qt5Svgd.dll

echo \bin64_debug\platforms\qwindowsd.dll
copy /Y %qtdir%\plugins\platforms\qwindowsd.dll %projectdir%bin64_debug\platforms\qwindowsd.dll

echo \bin64_debug\imageformats\qgifd.dll
copy /Y %qtdir%\plugins\imageformats\qgifd.dll %projectdir%bin64_debug\imageformats\qgifd.dll

echo \bin64_debug\imageformats\qjpegd.dll
copy /Y %qtdir%\plugins\imageformats\qjpegd.dll %projectdir%bin64_debug\imageformats\qjpegd.dll

echo \bin64_debug\imageformats\qtgad.dll
copy /Y %qtdir%\plugins\imageformats\qtgad.dll %projectdir%bin64_debug\imageformats\qtgad.dll

echo \bin64_debug\imageformats\qtiffd.dll
copy /Y %qtdir%\plugins\imageformats\qtiffd.dll %projectdir%bin64_debug\imageformats\qtiffd.dll

echo \bin64_debug\imageformats\qwebpd.dll
copy /Y %qtdir%\plugins\imageformats\qwebpd.dll %projectdir%bin64_debug\imageformats\qwebpd.dll

echo \bin64_debug\imageformats\qicod.dll
copy /Y %qtdir%\plugins\imageformats\qicod.dll %projectdir%bin64_debug\imageformats\qicod.dll

echo \bin64_debug\imageformats\qsvgd.dll
copy /Y %qtdir%\plugins\imageformats\qsvgd.dll %projectdir%bin64_debug\imageformats\qsvgd.dll

echo \bin64_debug\msvcp140d.dll
copy /Y %vcredistdird%\msvcp140d.dll %projectdir%bin64_debug\msvcp140d.dll

echo \bin64_debug\msvcp140_1d.dll
copy /Y %vcredistdird%\msvcp140_1d.dll %projectdir%bin64_debug\msvcp140_1d.dll

echo \bin64_debug\vcruntime140d.dll
copy /Y %vcredistdird%\vcruntime140d.dll %projectdir%bin64_debug\vcruntime140d.dll

echo \bin64_debug\ucrtbased.dll
copy /Y %ucrtdird%\ucrtbased.dll %projectdir%bin64_debug\ucrtbased.dll

pause