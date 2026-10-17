#ifndef GPU_DEBUGOUTPUT_H
#define GPU_DEBUGOUTPUT_H

namespace gpu
{
    // Prints GL debug messages to stderr, synchronously so a breakpoint lands on the faulty call
    bool enableDebugOutput();

    // Messages printed since startup or the last reset
    int debugMessageCount();
    void resetDebugMessageCount();
}

#endif
