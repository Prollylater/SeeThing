#ifndef DOC_LAYER_H
#define DOC_LAYER_H

#include "core/BlendMode.h"
#include "core/Geometry.h"
#include "core/Image.h"

#include <cstdint>
#include <string>

namespace doc
{
    using LayerId = std::uint32_t;

    enum class LayerKind
    {
        Raster,
        Result,
    };

    // What a properties step saves and restores
    struct LayerProps
    {
        std::string name;
        bool visible = true;
        bool locked = false;
        float opacity = 1.0f;
        core::BlendMode blend = core::BlendMode::Normal;

        friend bool operator==(const LayerProps &, const LayerProps &) = default;
    };

    class Layer
    {
    public:
        Layer() = default;
        // RGBA8, straight alpha
        Layer(std::string name, core::Image<std::uint8_t> pixels, LayerKind kind = LayerKind::Raster, std::string origin = {})
            : m_kind(kind), m_origin(std::move(origin)), m_pixels(std::move(pixels))
        {
            m_props.name = std::move(name);
        }

        LayerId id() const { return m_id; }
        LayerKind kind() const { return m_kind; }
        // Result layers: the algorithm and the parameters that made them
        const std::string &origin() const { return m_origin; }
        const LayerProps &props() const { return m_props; }
        // Layers inside a document are only reachable as const: change those with Document::setProps
        void setProps(LayerProps props) { m_props = std::move(props); }
        const core::Image<std::uint8_t> &pixels() const { return m_pixels; }
        int width() const { return m_pixels.width(); }
        int height() const { return m_pixels.height(); }

    private:
        friend class Document;

        LayerId m_id = 0;
        LayerKind m_kind = LayerKind::Raster;
        std::string m_origin;
        LayerProps m_props;
        core::Image<std::uint8_t> m_pixels;
        // Pixels changed since the renderer last took them
        core::Recti m_dirty;
    };
}

#endif
