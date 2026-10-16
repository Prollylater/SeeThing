#include "Canvas.h"

// Layer management

void Canvas::addLayer(Layer layer)
{
    layers.push_back(std::move(layer));
}
void Canvas::addAtLayer(Layer layer, int index)
{
    std::vector<Layer>::iterator pos = layers.begin() + index;
    layers.insert(pos, std::move(layer));
}
void Canvas::removeLayer(int index)
{
    std::vector<Layer>::iterator pos = layers.begin() + index;
    layers.erase(pos);
}
// at() throws on a bad index instead of reading past the vector
Layer &Canvas::getLayer(int index)
{
    return layers.at(index);
};
const Layer &Canvas::getLayer(int index) const
{
    return layers.at(index);
};

int Canvas::getLayerNb() const
{
    return layers.size();
};
