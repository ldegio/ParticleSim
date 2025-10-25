// Build as Windows subsystem without console, keep main() entry:
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#include "raylib.h"

int main()
{

}