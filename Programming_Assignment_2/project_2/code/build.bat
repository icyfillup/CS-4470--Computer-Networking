@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4505 -wd4530 -wd4201 -wd4100 -wd4189 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib wsock32.lib Ws2_32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% ..\project_2\code\chat.cpp /link %CommonLinkerFlags% 

popd
