
#include "UiCompUtils.h"
#include "AssetPath.h"
#include "ui/CanvasWidget.h"

#include <iostream>

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

    // Load the main font
    // io.Fonts->AddFontFromFileTTF(FONT_PATH_MAIN, 16.0f);
    // Explicit size: merging a sized font into an implicitly sized one asserts
    ImFontConfig base_cfg;
    base_cfg.SizePixels = 13.0f;
    base_cfg.PixelSnapH = true;
    io.Fonts->AddFontDefaultBitmap(&base_cfg);
    // Configure font merge
    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphMinAdvanceX = 14.0f;

    // Load the icon font and merge it
    io.Fonts->AddFontFromFileTTF(assetPath("fonts/codicon.ttf").c_str(), 16.0f, &config);
}

void ShowGenDisplayPanel(bool &show)
{
    static ui::CanvasView view;
    if (!show)
    {
        return;
    }
    if (ImGui::Begin("Segmentation Result", &show))
    {
        if (ImGui::Button("Save Result"))
        {
            std::string error;
            if (!appobj::glengine->saveResult("Lastdiplaysaved.jpg", &error))
                std::cerr << error << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            appobj::glengine->clearResult();
            show = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Transfer to Main"))
        {
            if (!appobj::glengine->transferResultToMain())
                std::cerr << "Transfer to Main: the result and the image differ in size" << std::endl;
            // Drop the result to stop hogging memory
            appobj::glengine->clearResult();
            show = false;
        }
        if (appobj::glengine->hasResult())
            ui::canvasWidget("result", appobj::glengine->resultTexture(), view);
    }
    ImGui::End();
}
