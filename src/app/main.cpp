#include "App.h"

#include <cstdio>
#include <exception>

int main()
{
    try
    {
        App app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "Fatal: %s\n", e.what());
        return 1;
    }
    return 0;
}
