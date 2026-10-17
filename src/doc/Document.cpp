#include "Document.h"

#include <algorithm>
#include <atomic>
#include <stdexcept>

namespace doc
{
    namespace
    {
        std::uint64_t nextSerial()
        {
            static std::atomic<std::uint64_t> serial{0};
            return ++serial;
        }
    }

    Document::Document(int width, int height)
        : m_w(width), m_h(height), m_serial(nextSerial()), m_selection(width, height, 1, core::ColorSpace::Mask)
    {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("doc::Document: empty size");
    }

    int Document::indexOf(LayerId id) const
    {
        for (int i = 0; i < layerCount(); ++i)
            if (m_layers[std::size_t(i)].m_id == id)
                return i;
        return -1;
    }

    const Layer *Document::find(LayerId id) const
    {
        const int i = indexOf(id);
        return i < 0 ? nullptr : &m_layers[std::size_t(i)];
    }

    Layer &Document::layerById(LayerId id)
    {
        const int i = indexOf(id);
        if (i < 0)
            throw std::out_of_range("doc::Document: no layer " + std::to_string(id));
        return m_layers[std::size_t(i)];
    }

    int Document::activeIndex() const
    {
        const int i = indexOf(m_active);
        return i >= 0 ? i : layerCount() - 1;
    }

    const Layer *Document::activeLayer() const
    {
        const int i = activeIndex();
        return i < 0 ? nullptr : &m_layers[std::size_t(i)];
    }

    void Document::setActive(int index)
    {
        m_active = layer(index).m_id;
        touch();
    }

    LayerId Document::insertLayer(int index, Layer layer)
    {
        if (layer.width() != m_w || layer.height() != m_h || layer.m_pixels.channels() != 4)
            throw std::invalid_argument("doc::Document: a layer must be RGBA at the document size");
        if (index < 0 || index > layerCount())
            throw std::out_of_range("doc::Document::insertLayer: bad index");
        if (layer.m_id == 0)
            layer.m_id = m_nextId++;
        else
            m_nextId = std::max(m_nextId, layer.m_id + 1);
        layer.m_dirty = bounds();
        const LayerId id = layer.m_id;
        m_layers.insert(m_layers.begin() + index, std::move(layer));
        m_active = id;
        touch();
        return id;
    }

    Layer Document::takeLayer(int index)
    {
        if (index < 0 || index >= layerCount())
            throw std::out_of_range("doc::Document::takeLayer: bad index");
        Layer layer = std::move(m_layers[std::size_t(index)]);
        m_layers.erase(m_layers.begin() + index);
        // The one below takes over, like in other editors
        if (m_active == layer.m_id && !m_layers.empty())
            m_active = m_layers[std::size_t(std::max(0, index - 1))].m_id;
        touch();
        return layer;
    }

    void Document::moveLayer(int from, int to)
    {
        if (from < 0 || from >= layerCount() || to < 0 || to >= layerCount())
            throw std::out_of_range("doc::Document::moveLayer: bad index");
        if (from < to)
            std::rotate(m_layers.begin() + from, m_layers.begin() + from + 1, m_layers.begin() + to + 1);
        else
            std::rotate(m_layers.begin() + to, m_layers.begin() + from, m_layers.begin() + from + 1);
        touch();
    }

    void Document::setProps(int index, const LayerProps &props)
    {
        m_layers.at(std::size_t(index)).m_props = props;
        touch();
    }

    core::Image<std::uint8_t> &Document::pixelsForEdit(LayerId id)
    {
        return layerById(id).m_pixels;
    }

    void Document::pixelsChanged(LayerId id, core::Recti rect)
    {
        Layer &l = layerById(id);
        l.m_dirty = core::unite(l.m_dirty, core::intersect(rect, bounds()));
        touch();
    }

    core::Recti Document::takeDirty(LayerId id)
    {
        return std::exchange(layerById(id).m_dirty, core::Recti{});
    }

    void Document::selectionChanged(core::Recti rect)
    {
        m_selectionDirty = core::unite(m_selectionDirty, core::intersect(rect, bounds()));
        m_hasSelection = std::any_of(m_selection.data(), m_selection.data() + m_selection.size(), [](std::uint8_t v) { return v != 0; });
        touch();
    }

    void Document::setSeeds(std::vector<core::Vec2i> seeds)
    {
        m_seeds = std::move(seeds);
        touch();
    }

    core::Recti Document::takeSelectionDirty()
    {
        return std::exchange(m_selectionDirty, core::Recti{});
    }
}
