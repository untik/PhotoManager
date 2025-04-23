@echo off

if %1.==. goto DEFAULT_FILE
set projectArchiveName=%1
goto ARCHIVE

:DEFAULT_FILE
set cmd="update-version.bat -print"
FOR /F "tokens=*" %%i IN (' %cmd% ') DO SET VERSION=%%i
set projectArchiveName=PhotoManager-%VERSION%.zip
goto ARCHIVE

:ARCHIVE
del %projectArchiveName%
cd bin64
"c:\Program Files\7-Zip\7z.exe" a ..\%projectArchiveName% "-ir!*" -mmt
cd ..
