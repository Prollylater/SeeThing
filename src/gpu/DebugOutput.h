#ifndef GPU_DEBUGOUTPUT_H
#define GPU_DEBUGOUTPUT_H

namespace core
{
    class Log;
}

namespace gpu
{
    // Reports GL debug messages to log (stderr without one), synchronously so a breakpoint lands on the faulty call.
    // The log must outlive the context.
    bool enableDebugOutput(core::Log *log = nullptr);

    // Messages printed since startup or the last reset
    int debugMessageCount();
    void resetDebugMessageCount();
}

#endif
