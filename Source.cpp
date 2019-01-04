// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include <Engine/Application/Application.h>
#include <Engine/Profiling/Logging.h>

#ifdef ConsoleWindowDisabled
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

// void main()
// {
// 	CApplication Application;
// 	Application.Run();
// }
