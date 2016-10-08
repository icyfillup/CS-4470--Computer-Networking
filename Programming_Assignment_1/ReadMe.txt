Philip Tran - 302286600
Allen Huang - 302348441

Locations of files:
	chat.exe - Inside build folder.
	chat.cpp - Inside project_1 >> code folder.

> This chat.exe program is a Windows application.
> Please running the chat.exe program on console or command line.
> The chat.exe program is in the build folder. 
> To run the program on command line, type chat.exe followed by the port number of your choosing. Port number above 3000. (chat.exe [port #])
	-Example: chat.exe 5555

	
> The chat.cpp file is under [project_1 >> code >> chat.cpp].
> If you want to compile the chat.cpp file, run the build.bat which is in the code folder. 
> In the build.bat file, it has the compile call and switches to chat.cpp.
> When you run the build.bat file, the chat.exe will be made and located in the build folder.

> WARNING: Using the build.bat assumes that the computer has Microsoft Visual Studio 12.0 cl.exe compiler in the ..\Program Files (x86) folder.

> If you want to compile chat.cpp by yourself, make sure the computer has  Microsoft Visual Studio 12.0 cl.exe compiler installed.
> [NOTE]: wsock32.lib and Ws2_32.lib must be linked while compiling.