
#ifndef _GK_GL3CORE_H

#ifdef GK_MACOS
    #include <OpenGL/gl3.h>
    #define NO_GLEW
#else
    // windows et linux
    #define GLEW_NO_GLU
    #include "GL/glew.h"
    //#include "imgui_impl_opengl3_loader.h"
    #ifdef WIN32
    #  ifdef defined(_MSC_VER)   // vstudio
    #    define DEBUGCALLBACK 
    #  endif
    #else                       // linux
    #  define DEBUGCALLBACK 
    #endif
#endif

#endif
