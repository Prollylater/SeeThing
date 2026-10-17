#ifndef DOC_DOCUMENT_H
#define DOC_DOCUMENT_H

#include "History.h"
#include "Layer.h"

#include <cstdint>
#include <vector>

namespace doc
{
    // The pixels the algorithms read (the CPU copy is the truth, the GPU mirrors it), the layer stack,
    // the selection and the history.
    // The mutators below are raw: they record nothing. Undoable changes go through Edits.h.
    class Document
    {
    public:
        Document() = default;
        Document(int width, int height);

        bool empty() const { return m_w == 0; }
        int width() const { return m_w; }
        int height() const { return m_h; }
        core::Recti bounds() const { return {0, 0, m_w, m_h}; }
        // Tells two documents apart even when they reuse layer ids
        std::uint64_t serial() const { return m_serial; }
        // Bumped by every change; the renderer recomposites when it moves
        std::uint64_t revision() const { return m_revision; }

        // Bottom to top
        int layerCount() const { return int(m_layers.size()); }
        const Layer &layer(int index) const { return m_layers.at(std::size_t(index)); }
        // -1 when there is no such layer
        int indexOf(LayerId id) const;
        const Layer *find(LayerId id) const;

        // -1 when there are no layers
        int activeIndex() const;
        const Layer *activeLayer() const;
        void setActive(int index);

        // Gives the layer a new id when it has none; throws when its size isn't the document's
        LayerId insertLayer(int index, Layer layer);
        Layer takeLayer(int index);
        void moveLayer(int from, int to);
        void setProps(int index, const LayerProps &props);

        // Write through pixelsForEdit, then report the rectangle with pixelsChanged
        core::Image<std::uint8_t> &pixelsForEdit(LayerId id);
        void pixelsChanged(LayerId id, core::Recti rect);
        // The rectangle changed since the last call, cleared by the call
        core::Recti takeDirty(LayerId id);

        // One channel, document size; 0 is unselected. Nothing selected means everything is editable.
        const core::Image<std::uint8_t> &selection() const { return m_selection; }
        core::Image<std::uint8_t> &selectionForEdit() { return m_selection; }
        void selectionChanged(core::Recti rect);
        core::Recti takeSelectionDirty();
        bool hasSelection() const { return m_hasSelection; }

        // Pixel positions the seeded region growing starts from
        const std::vector<core::Vec2i> &seeds() const { return m_seeds; }
        void setSeeds(std::vector<core::Vec2i> seeds);

        History &history() { return m_history; }
        const History &history() const { return m_history; }
        void undo() { m_history.undo(*this); }
        void redo() { m_history.redo(*this); }

    private:
        Layer &layerById(LayerId id);
        void touch() { ++m_revision; }

        int m_w = 0, m_h = 0;
        std::uint64_t m_serial = 0;
        std::uint64_t m_revision = 0;
        std::vector<Layer> m_layers;
        LayerId m_nextId = 1;
        LayerId m_active = 0;
        core::Image<std::uint8_t> m_selection;
        core::Recti m_selectionDirty;
        bool m_hasSelection = false;
        std::vector<core::Vec2i> m_seeds;
        History m_history;
    };
}

#endif
