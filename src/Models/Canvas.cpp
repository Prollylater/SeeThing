#include "Canvas.h"

// Layer management

void Canvas::addLayer(Layer &layer)
{
    layers.push_back(layer);
}
void Canvas::addAtLayer(Layer &layer, int index)
{
    std::vector<Layer>::iterator pos = layers.begin() + index;
    layers.insert(pos, layer);
}
void Canvas::removeLayer(int index)
{
    std::vector<Layer>::iterator pos = layers.begin() + index;
    layers.erase(pos);
}
Layer &Canvas::getLayer(int index)
{
    return layers[index];
};
const Layer &Canvas::getLayer(int index) const
{
    return layers[index];
};

int Canvas::getLayerNb() const
{
    return layers.size();
};
