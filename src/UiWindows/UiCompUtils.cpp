
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

/*
void SetupGlEngine()
{
    if (!appobj::glengine.initImrender())
    {
        std::cerr << "Buffers Initialization failed" << std::endl;
    }
}*/