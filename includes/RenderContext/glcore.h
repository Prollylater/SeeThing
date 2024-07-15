
#ifndef GL3CORE_H
#define GL3CORE_H

    // windows et linux
    #define GLEW_NO_GLU
    #include "GL/glew.h"
    #include <GLFW/glfw3.h>
    

    //#include "imgui_impl_opengl3_loader.h"
    #ifdef WIN32
    #  ifdef defined(_MSC_VER)   // vstudio
    #    define DEBUGCALLBACK 
    #  endif
    #else                       // linux
    #  define DEBUGCALLBACK 
    #endif


#endif
