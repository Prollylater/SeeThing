#include "Edits.h"

#include "core/Color.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace doc
{
    namespace
    {
        using Pixels = core::Image<std::uint8_t>;

        Pixels copyOut(const Pixels &img, core::Recti r)
        {
            const int c = img.channels();
            Pixels out(r.w, r.h, c, img.colorSpace());
            for (int y = 0; y < r.h; ++y)
                std::copy_n(img.row(r.y + y) + std::size_t(r.x) * c, std::size_t(r.w) * c, out.row(y));
            return out;
        }

        void pasteIn(Pixels &img, core::Recti r, const Pixels &tile)
        {
            const int c = img.channels();
            for (int y = 0; y < r.h; ++y)
                std::copy_n(tile.row(y), std::size_t(r.w) * c, img.row(r.y + y) + std::size_t(r.x) * c);
        }

        bool samePixels(const Pixels &a, const Pixels &b)
        {
            return std::equal(a.data(), a.data() + a.size(), b.data());
        }

        Pixels &targetImage(Document &doc, LayerId target)
        {
            return target == selectionTarget ? doc.selectionForEdit() : doc.pixelsForEdit(target);
        }

        void notify(Document &doc, LayerId target, core::Recti rect)
        {
            if (target == selectionTarget)
                doc.selectionChanged(rect);
            else
                doc.pixelsChanged(target, rect);
        }

        class PixelEdit : public Command
        {
        public:
            struct Tile
            {
                core::Recti rect;
                Pixels before, after;
            };

            PixelEdit(LayerId target, std::string label, std::vector<Tile> tiles)
                : m_target(target), m_label(std::move(label)), m_tiles(std::move(tiles))
            {
                for (const Tile &t : m_tiles)
                    m_bytes += t.before.size() + t.after.size();
            }

            std::string label() const override { return m_label; }
            std::size_t bytes() const override { return m_bytes; }
            void undo(Document &doc) override { apply(doc, &Tile::before); }
            void redo(Document &doc) override { apply(doc, &Tile::after); }

        private:
            void apply(Document &doc, Pixels Tile::*state)
            {
                Pixels &img = targetImage(doc, m_target);
                core::Recti changed;
                for (const Tile &t : m_tiles)
                {
                    pasteIn(img, t.rect, t.*state);
                    changed = core::unite(changed, t.rect);
                }
                notify(doc, m_target, changed);
            }

            LayerId m_target;
            std::string m_label;
            std::vector<Tile> m_tiles;
            std::size_t m_bytes = 0;
        };

        class InsertLayer : public Command
        {
        public:
            InsertLayer(Document &doc, int index, LayerId id, LayerId previousActive, std::string label)
                : m_index(index), m_id(id), m_previousActive(previousActive), m_label(std::move(label)),
                  m_bytes(doc.layer(index).pixels().size()) {}

            std::string label() const override { return m_label; }
            std::size_t bytes() const override { return m_bytes; }
            void undo(Document &doc) override
            {
                m_layer = doc.takeLayer(doc.indexOf(m_id));
                if (const int i = doc.indexOf(m_previousActive); i >= 0)
                    doc.setActive(i);
            }
            void redo(Document &doc) override { doc.insertLayer(m_index, std::move(m_layer)); }

        private:
            int m_index;
            LayerId m_id, m_previousActive;
            std::string m_label;
            std::size_t m_bytes;
            Layer m_layer; // held while undone
        };

        class RemoveLayer : public Command
        {
        public:
            RemoveLayer(int index, Layer layer)
                : m_index(index), m_id(layer.id()), m_label("Delete " + layer.props().name), m_layer(std::move(layer)) {}

            std::string label() const override { return m_label; }
            std::size_t bytes() const override { return m_layer.pixels().size(); }
            void undo(Document &doc) override { doc.insertLayer(m_index, std::move(m_layer)); }
            void redo(Document &doc) override { m_layer = doc.takeLayer(doc.indexOf(m_id)); }

        private:
            int m_index;
            LayerId m_id;
            std::string m_label;
            Layer m_layer; // held while applied
        };

        class MoveLayer : public Command
        {
        public:
            MoveLayer(int from, int to, std::string label) : m_from(from), m_to(to), m_label(std::move(label)) {}

            std::string label() const override { return m_label; }
            void undo(Document &doc) override { doc.moveLayer(m_to, m_from); }
            void redo(Document &doc) override { doc.moveLayer(m_from, m_to); }

        private:
            int m_from, m_to;
            std::string m_label;
        };

        class SetProps : public Command
        {
        public:
            SetProps(LayerId id, LayerProps before, LayerProps after, std::string label)
                : m_id(id), m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label)) {}

            std::string label() const override { return m_label; }
            void undo(Document &doc) override { doc.setProps(doc.indexOf(m_id), m_before); }
            void redo(Document &doc) override { doc.setProps(doc.indexOf(m_id), m_after); }

        private:
            LayerId m_id;
            LayerProps m_before, m_after;
            std::string m_label;
        };

        class SetSeeds : public Command
        {
        public:
            SetSeeds(std::vector<core::Vec2i> before, std::vector<core::Vec2i> after, std::string label)
                : m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label)) {}

            std::string label() const override { return m_label; }
            std::size_t bytes() const override { return (m_before.size() + m_after.size()) * sizeof(core::Vec2i); }
            void undo(Document &doc) override { doc.setSeeds(m_before); }
            void redo(Document &doc) override { doc.setSeeds(m_after); }

        private:
            std::vector<core::Vec2i> m_before, m_after;
            std::string m_label;
        };

        void changeSeeds(Document &doc, std::vector<core::Vec2i> after, std::string label)
        {
            std::vector<core::Vec2i> before = doc.seeds();
            doc.setSeeds(after);
            doc.history().record(std::make_unique<SetSeeds>(std::move(before), std::move(after), std::move(label)));
        }

        template <class F>
        bool editSelection(Document &doc, const char *label, F &&change)
        {
            PixelEditRecorder rec(doc, selectionTarget);
            rec.touch(doc.bounds());
            Pixels &mask = rec.image();
            std::uint8_t *px = mask.data();
            for (std::size_t i = 0; i < mask.size(); ++i)
                px[i] = change(px[i]);
            return rec.commit(label);
        }
    }

    PixelEditRecorder::PixelEditRecorder(Document &doc, LayerId target)
        : m_doc(doc), m_target(target), m_image(&targetImage(doc, target))
    {
    }

    void PixelEditRecorder::touch(core::Recti rect)
    {
        rect = core::intersect(rect, m_doc.bounds());
        if (rect.empty())
            return;
        m_touched = core::unite(m_touched, rect);
        for (int ty = rect.y / tileSize; ty <= (rect.bottom() - 1) / tileSize; ++ty)
            for (int tx = rect.x / tileSize; tx <= (rect.right() - 1) / tileSize; ++tx)
                if (!m_before.contains({tx, ty}))
                    m_before.emplace(std::pair{tx, ty}, copyOut(*m_image, core::intersect({tx * tileSize, ty * tileSize, tileSize, tileSize}, m_doc.bounds())));
    }

    bool PixelEditRecorder::commit(std::string label)
    {
        std::vector<PixelEdit::Tile> tiles;
        for (auto &[key, before] : m_before)
        {
            const core::Recti r = core::intersect({key.first * tileSize, key.second * tileSize, tileSize, tileSize}, m_doc.bounds());
            Pixels after = copyOut(*m_image, r);
            if (!samePixels(before, after))
                tiles.push_back({r, std::move(before), std::move(after)});
        }
        m_before.clear();
        notify(m_doc, m_target, std::exchange(m_touched, core::Recti{}));
        if (tiles.empty())
            return false;
        m_doc.history().record(std::make_unique<PixelEdit>(m_target, std::move(label), std::move(tiles)));
        return true;
    }

    LayerId addLayer(Document &doc, Layer layer, int index, std::string label)
    {
        if (index < 0)
            index = doc.activeIndex() + 1;
        const LayerId previous = doc.activeLayer() ? doc.activeLayer()->id() : 0;
        const LayerId id = doc.insertLayer(index, std::move(layer));
        doc.history().record(std::make_unique<InsertLayer>(doc, index, id, previous, std::move(label)));
        return id;
    }

    LayerId addEmptyLayer(Document &doc)
    {
        Pixels clear(doc.width(), doc.height(), 4);
        return addLayer(doc, Layer("Layer " + std::to_string(doc.layerCount() + 1), std::move(clear)), -1, "New Layer");
    }

    LayerId duplicateLayer(Document &doc, int index)
    {
        const Layer &src = doc.layer(index);
        Layer copy(src.props().name + " copy", src.pixels(), src.kind(), src.origin());
        LayerProps props = src.props();
        props.name = src.props().name + " copy";
        copy.setProps(std::move(props));
        return addLayer(doc, std::move(copy), index + 1, "Duplicate " + src.props().name);
    }

    void removeLayer(Document &doc, int index)
    {
        Layer layer = doc.takeLayer(index);
        doc.history().record(std::make_unique<RemoveLayer>(index, std::move(layer)));
    }

    void moveLayer(Document &doc, int from, int to)
    {
        if (from == to || to < 0 || to >= doc.layerCount())
            return;
        const std::string label = std::string(to > from ? "Raise " : "Lower ") + doc.layer(from).props().name;
        doc.moveLayer(from, to);
        doc.history().record(std::make_unique<MoveLayer>(from, to, label));
    }

    void setLayerProps(Document &doc, int index, const LayerProps &props, std::string label)
    {
        const LayerProps before = doc.layer(index).props();
        if (before == props)
            return;
        doc.setProps(index, props);
        doc.history().record(std::make_unique<SetProps>(doc.layer(index).id(), before, props, std::move(label)));
    }

    void recordLayerProps(Document &doc, int index, const LayerProps &before, std::string label)
    {
        const Layer &l = doc.layer(index);
        if (before != l.props())
            doc.history().record(std::make_unique<SetProps>(l.id(), before, l.props(), std::move(label)));
    }

    bool replacePixels(Document &doc, LayerId id, core::ImageView<const std::uint8_t> image, std::string label)
    {
        if (image.width() != doc.width() || image.height() != doc.height())
            throw std::invalid_argument("doc::replacePixels: the image size isn't the document's");
        const Pixels rgba = core::toRGBA8(image);
        PixelEditRecorder rec(doc, id);
        rec.touch(doc.bounds());
        std::copy_n(rgba.data(), rgba.size(), rec.image().data());
        return rec.commit(std::move(label));
    }

    bool fillMask(Document &doc, LayerId id, const Pixels &mask, core::Recti bounds, std::array<std::uint8_t, 4> rgba, std::string label)
    {
        bounds = core::intersect(bounds, doc.bounds());
        if (bounds.empty() || mask.width() != doc.width() || mask.height() != doc.height())
            return false;
        const Pixels &sel = doc.selection();
        const bool clip = doc.hasSelection();
        PixelEditRecorder rec(doc, id);
        rec.touch(bounds);
        Pixels &px = rec.image();
        for (int y = bounds.y; y < bounds.bottom(); ++y)
            for (int x = bounds.x; x < bounds.right(); ++x)
                if (mask(x, y) >= 128 && (!clip || sel(x, y) >= 128))
                    for (int k = 0; k < 4; ++k)
                        px(x, y, k) = rgba[std::size_t(k)];
        return rec.commit(std::move(label));
    }

    void addSeed(Document &doc, core::Vec2i p)
    {
        if (p.x < 0 || p.y < 0 || p.x >= doc.width() || p.y >= doc.height())
            return;
        std::vector<core::Vec2i> seeds = doc.seeds();
        if (std::find(seeds.begin(), seeds.end(), p) != seeds.end())
            return;
        seeds.push_back(p);
        changeSeeds(doc, std::move(seeds), "Add seed");
    }

    bool removeSeedNear(Document &doc, core::Vec2f p, float radius)
    {
        std::vector<core::Vec2i> seeds = doc.seeds();
        auto best = seeds.end();
        float bestDist = radius * radius;
        for (auto it = seeds.begin(); it != seeds.end(); ++it)
        {
            const float dx = it->x + 0.5f - p.x, dy = it->y + 0.5f - p.y;
            if (dx * dx + dy * dy <= bestDist)
            {
                bestDist = dx * dx + dy * dy;
                best = it;
            }
        }
        if (best == seeds.end())
            return false;
        seeds.erase(best);
        changeSeeds(doc, std::move(seeds), "Remove seed");
        return true;
    }

    void clearSeeds(Document &doc)
    {
        if (!doc.seeds().empty())
            changeSeeds(doc, {}, "Clear seeds");
    }

    bool selectAll(Document &doc)
    {
        return editSelection(doc, "Select All", [](std::uint8_t) { return std::uint8_t(255); });
    }

    bool selectNone(Document &doc)
    {
        return editSelection(doc, "Deselect", [](std::uint8_t) { return std::uint8_t(0); });
    }

    bool invertSelection(Document &doc)
    {
        return editSelection(doc, "Invert Selection", [](std::uint8_t v) { return std::uint8_t(255 - v); });
    }
}
