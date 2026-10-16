#ifndef CANVAS_H
#define CANVAS_H
#include "Layer.h"
#include <vector>

class Canvas
{
private:
    std::vector<Layer> layers;

public:
    // Handle size

    Canvas() = default;
    // TODO remove remnant of singleton patter here
    Canvas(const Canvas &) = delete;
    Canvas &operator=(const Canvas &) = delete;
    // Layer management
    void addLayer(Layer layer);
    void addAtLayer(Layer layer, int index);
    void removeLayer(int index);
    Layer &getLayer(int index);
    const Layer &getLayer(int index) const;
    int getLayerNb() const;
    // Undo/Redo management
};

// int width;
// int height;
//  Canvas(int width = 1024, int height = 576);

#endif
