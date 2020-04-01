@echo off

IF NOT EXIST build mkdir build
pushd build
cl /Zi /WX ../source/win32_crest.c /link user32.lib gdi32.lib opengl32.lib winmm.lib /out:crest.exe
popd
