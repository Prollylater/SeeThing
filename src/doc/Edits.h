#ifndef DOC_EDITS_H
#define DOC_EDITS_H

#include "Document.h"

#include <array>
#include <map>
#include <string>
#include <utility>

// Undoable changes: each applies itself and records one history step.
namespace doc
{
    // The target of a pixel edit that isn't a layer
    inline constexpr LayerId selectionTarget = 0;

    // Records a pixel edit as 64 x 64 tiles: touch() saves the tiles a rectangle covers before you change them,
    // commit() saves them again and records the tiles that differ as one step.
    class PixelEditRecorder
    {
    public:
        static constexpr int tileSize = 64;

        PixelEditRecorder(Document &doc, LayerId target);
        core::Image<std::uint8_t> &image() { return *m_image; }
        void touch(core::Recti rect);
        // False, and no step, when no pixel changed
        bool commit(std::string label);

    private:
        Document &m_doc;
        LayerId m_target;
        core::Image<std::uint8_t> *m_image;
        std::map<std::pair<int, int>, core::Image<std::uint8_t>> m_before;
        core::Recti m_touched;
    };

    // index -1 puts it above the active layer. It becomes the active one.
    LayerId addLayer(Document &doc, Layer layer, int index, std::string label);
    LayerId addEmptyLayer(Document &doc);
    LayerId duplicateLayer(Document &doc, int index);
    void removeLayer(Document &doc, int index);
    void moveLayer(Document &doc, int from, int to);
    void setLayerProps(Document &doc, int index, const LayerProps &props, std::string label);
    // For a change already made live (a slider drag): records before -> the current props as one step
    void recordLayerProps(Document &doc, int index, const LayerProps &before, std::string label);
    // Same size as the document; converted to RGBA
    bool replacePixels(Document &doc, LayerId id, core::ImageView<const std::uint8_t> image, std::string label);

    // Where mask is at least one half, and inside the selection when there is one; bounds holds the mask
    bool fillMask(Document &doc, LayerId id, const core::Image<std::uint8_t> &mask, core::Recti bounds,
                  std::array<std::uint8_t, 4> rgba, std::string label);

    void addSeed(Document &doc, core::Vec2i p);
    // The seed nearest to p within radius pixels; false when there is none
    bool removeSeedNear(Document &doc, core::Vec2f p, float radius);
    void clearSeeds(Document &doc);

    bool selectAll(Document &doc);
    bool selectNone(Document &doc);
    bool invertSelection(Document &doc);
}

#endif
