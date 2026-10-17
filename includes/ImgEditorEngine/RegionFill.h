#ifndef REGIONFILL_H
#define REGIONFILL_H

#include "core/Image.h"
#include "doc/Selection.h"

// Seeded region growing from one pixel, for the bucket and the wand.
// compare is an index into rgs_comp_func_var, as in the Seeded tab. With a limit (R8, the image size),
// the region never enters pixels under 128 there, and a seed outside it gives an empty region.
doc::Shape growRegion(core::ImageView<const std::uint8_t> rgb, core::Vec2i seed, float threshold, int compare,
                      const core::Image<std::uint8_t> *limit = nullptr);

#endif
