#ifndef ASSETPATH_H
#define ASSETPATH_H
#include <string>

// File under ressources/: $SEETHING_ASSET_DIR, else next to the executable, else ./ressources
std::string assetPath(const std::string &relative);

#endif
