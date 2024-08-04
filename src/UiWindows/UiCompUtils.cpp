
#include "UiCompUtils.h"

void ShowColorPicker(bool &show, ImVec4 &color)
{
    if (!show)
    {
        return;
    }

    // Todo Style

    if (ImGui::Begin("Color Window", &show))
    {

        ImGui::ColorEdit3("Col", (float *)&color);
    }

    ImGui::End();
}

void SetupImGuiFonts()
{
    ImGuiIO &io = ImGui::GetIO();
    static const ImWchar icon_ranges[] = {ICON_MIN_CI, ICON_MAX_CI, 0};

    // Load the main font
    // io.Fonts->AddFontFromFileTTF(FONT_PATH_MAIN, 16.0f);
    io.Fonts->AddFontDefault();
    // Configure font merge
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = 14.0f;

    // Load the icon font and merge it
    io.Fonts->AddFontFromFileTTF("./ressources/fonts/codicon.ttf", 16.0f, &config, icon_ranges);

    // Build the font atlas
    io.Fonts->Build();
}

void ShowGenDisplayPanel(bool &show, GLuint &tex)
{
    if (!show)
    {
        return;
    }
    int width = appobj::canvas.getLayer(0).getWidth();
    int height = appobj::canvas.getLayer(0).getHeight();
    if (ImGui::Begin("Segmentation Mode", &show))
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImVec2 parent_size = ImGui::GetWindowSize();
    float button_size= 20.0f;
    ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - button_size, 1.0f));
    ImGui::Button("Save Result",ImVec2(button_size, 1.0f));
    ImGui::SameLine();

    ImGui::End();
}
