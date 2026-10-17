#include "RegionFill.h"

#include "MatBridge.h"
#include "RegionGrowingAlg.h"

#include <memory>
#include <queue>
#include <vector>

doc::Shape growRegion(core::ImageView<const std::uint8_t> rgb, core::Vec2i seed, float threshold, int compare,
                      const core::Image<std::uint8_t> *limit)
{
    doc::Shape shape{core::Image<std::uint8_t>(rgb.width(), rgb.height(), 1, core::ColorSpace::Mask), {}};
    if (seed.x < 0 || seed.y < 0 || seed.x >= rgb.width() || seed.y >= rgb.height())
        return shape;
    if (limit && (*limit)(seed.x, seed.y) < 128)
        return shape;
    const Mat<uint8_t> image = toMat(rgb);
    const Mat<uint8_t> roi = limit ? toMat(*limit) : Mat<uint8_t>();
    std::vector<Region> regions(1);
    std::shared_ptr<Pixl> germ(new Pixl(seed.x, seed.y, 0));
    std::queue<std::shared_ptr<Pixl>> queue;
    queue.push(germ);
    // addGerm moves the pointer out of its argument: queue it first
    regions[0].addGerm(germ, image);
    Mat<uint32_t> visited(image.getRows(), image.getCols(), 1);
    regionGrowing3c(image, queue, threshold, visited, regions, compare, limit ? &roi : nullptr);

    for (int y = 0; y < rgb.height(); ++y)
        for (int x = 0; x < rgb.width(); ++x)
            if (visited.atChannel(y, x, 0) == 0)
            {
                shape.mask(x, y) = 255;
                shape.bounds = core::unite(shape.bounds, {x, y, 1, 1});
            }
    return shape;
}
