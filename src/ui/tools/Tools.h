#ifndef UI_TOOLS_H
#define UI_TOOLS_H

#include "Tool.h"

#include <memory>
#include <vector>

namespace ui
{
    // Every tool, in toolbox order
    std::vector<std::unique_ptr<Tool>> makeTools();
}

#endif
