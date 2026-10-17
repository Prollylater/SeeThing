#include "LayersPanel.h"

#include "IconsCodicons.h"
#include "app/App.h"
#include "doc/Edits.h"

#include "imgui.h"

#include <cstdio>

namespace ui
{
    namespace
    {
        bool iconButton(const char *icon, const char *tooltip, bool enabled = true)
        {
            ImGui::BeginDisabled(!enabled);
            const bool clicked = ImGui::Button(icon);
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s", tooltip);
            return clicked;
        }
    }

    void LayersPanel::draw(App &app, bool &open)
    {
        if (!open)
            return;
        if (!ImGui::Begin(name, &open))
        {
            ImGui::End();
            return;
        }
        if (!app.hasImage())
        {
            ImGui::TextDisabled("No image.");
            ImGui::End();
            return;
        }

        doc::Document &doc = app.document();
        const int active = doc.activeIndex();
        if (iconButton(ICON_CI_ADD "##new", "New transparent layer above the active one"))
            doc::addEmptyLayer(doc);
        ImGui::SameLine();
        if (iconButton(ICON_CI_COPY "##duplicate", "Duplicate the active layer", active >= 0))
            doc::duplicateLayer(doc, active);
        ImGui::SameLine();
        if (iconButton(ICON_CI_TRASH "##delete", "Delete the active layer", doc.layerCount() > 1))
            doc::removeLayer(doc, active);
        ImGui::SameLine();
        if (iconButton(ICON_CI_ARROW_UP "##raise", "Move up", active + 1 < doc.layerCount()))
            doc::moveLayer(doc, active, active + 1);
        ImGui::SameLine();
        if (iconButton(ICON_CI_ARROW_DOWN "##lower", "Move down", active > 0))
            doc::moveLayer(doc, active, active - 1);

        drawProperties(app);
        ImGui::Separator();
        drawList(app);
        ImGui::End();
    }

    void LayersPanel::drawProperties(App &app)
    {
        doc::Document &doc = app.document();
        const int index = doc.activeIndex();
        if (index < 0)
            return;
        const doc::LayerProps props = doc.layer(index).props();

        if (ImGui::BeginCombo("Blend", core::blendModeName(props.blend)))
        {
            for (core::BlendMode mode : core::blendModes)
                if (ImGui::Selectable(core::blendModeName(mode), mode == props.blend) && mode != props.blend)
                {
                    doc::LayerProps p = props;
                    p.blend = mode;
                    doc::setLayerProps(doc, index, p, std::string("Blend ") + core::blendModeName(mode));
                }
            ImGui::EndCombo();
        }
        ImGui::SetItemTooltip("Blending happens on sRGB values, as in most editors");

        // Applied live while dragging, recorded once on release
        float percent = props.opacity * 100.0f;
        if (ImGui::SliderFloat("Opacity", &percent, 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp))
        {
            doc::LayerProps p = props;
            p.opacity = percent / 100.0f;
            doc.setProps(index, p);
        }
        if (ImGui::IsItemActivated())
            m_dragStart = props;
        if (ImGui::IsItemDeactivatedAfterEdit())
            doc::recordLayerProps(doc, index, m_dragStart, "Opacity " + doc.layer(index).props().name);

        bool locked = props.locked;
        if (ImGui::Checkbox("Locked", &locked))
        {
            doc::LayerProps p = props;
            p.locked = locked;
            doc::setLayerProps(doc, index, p, (locked ? "Lock " : "Unlock ") + props.name);
        }
        ImGui::SetItemTooltip("A locked layer can't be painted on or replaced");
    }

    void LayersPanel::drawList(App &app)
    {
        doc::Document &doc = app.document();
        if (!ImGui::BeginChild("##layers", ImVec2(0, 0), ImGuiChildFlags_None))
        {
            ImGui::EndChild();
            return;
        }
        const int active = doc.activeIndex();
        for (int i = doc.layerCount() - 1; i >= 0; --i)
        {
            const doc::Layer &layer = doc.layer(i);
            const doc::LayerProps &props = layer.props();
            ImGui::PushID(int(layer.id()));

            if (ImGui::SmallButton(props.visible ? ICON_CI_EYE : ICON_CI_EYE_CLOSED))
            {
                doc::LayerProps p = props;
                p.visible = !p.visible;
                doc::setLayerProps(doc, i, p, (p.visible ? "Show " : "Hide ") + props.name);
            }
            ImGui::SetItemTooltip(props.visible ? "Hide" : "Show");
            ImGui::SameLine();

            std::string label = props.name;
            if (props.locked)
                label += "  " ICON_CI_LOCK;
            if (layer.kind() == doc::LayerKind::Result)
                label += "  (result)";
            if (ImGui::Selectable(label.c_str(), i == active, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (i != active)
                    doc.setActive(i);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    m_renaming = layer.id();
                    std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", props.name.c_str());
                    ImGui::OpenPopup("##rename");
                }
            }
            if (!layer.origin().empty())
                ImGui::SetItemTooltip("%s", layer.origin().c_str());
            else
                ImGui::SetItemTooltip("Double-click to rename");
            // The popup belongs to this row's ID scope
            if (m_renaming == layer.id())
                drawRenamePopup(app);
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    void LayersPanel::drawRenamePopup(App &app)
    {
        if (!ImGui::BeginPopup("##rename"))
            return;
        doc::Document &doc = app.document();
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();
        const bool enter = ImGui::InputText("Name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
        if (enter || ImGui::Button("OK"))
        {
            const int index = doc.indexOf(m_renaming);
            if (index >= 0 && m_renameBuffer[0] != '\0')
            {
                doc::LayerProps p = doc.layer(index).props();
                p.name = m_renameBuffer;
                doc::setLayerProps(doc, index, p, "Rename " + doc.layer(index).props().name);
            }
            m_renaming = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
