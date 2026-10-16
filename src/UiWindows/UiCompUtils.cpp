
#include "UiCompUtils.h"
#include "AssetPath.h"

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

void ImageTexSampler(GLuint tex, const ImVec2 &size)
{
    // The backend's linear sampler would override the texture's own GL_NEAREST
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
    if (platform_io.DrawCallback_SetSamplerFromTex)
        draw_list->AddCallback(platform_io.DrawCallback_SetSamplerFromTex);
    ImGui::Image((ImTextureID)(intptr_t)tex, size, ImVec2(0, 1), ImVec2(1, 0));
    if (platform_io.DrawCallback_ResetRenderState)
        draw_list->AddCallback(platform_io.DrawCallback_ResetRenderState);
}

void ShowGenDisplayPanel(bool &show, GLuint &tex)
{

    if (!show)
    {
        return;
    }
    int width = appobj::canvas.getLayer(0).getWidth();
    int height = appobj::canvas.getLayer(0).getHeight();
    if (ImGui::Begin("Segmentation Result", &show))
    {
        ImageTexSampler(tex, ImVec2(width, height));
    }
    ImVec2 parent_size = ImGui::GetWindowSize();
    float button_size = 20.0f;
    ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - button_size, 1.0f));
    const char *save_text = "Save Result";
    ImVec2 save_text_size = ImGui::CalcTextSize(save_text);
    if (ImGui::Button(save_text, ImVec2(save_text_size.x + button_size, save_text_size.y + button_size)))
    {
        appobj::glengine.saveTextInst(tex, "Lastdiplaysaved.jpg");
    };
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(80.0f, 20.0f)))
    {
        appobj::glengine.deleteTexture(tex);
        show = false;
    };
    ImGui::SameLine();
    const char *transfer_text = "Transfer to Main";
    ImVec2 trans_text_size = ImGui::CalcTextSize(transfer_text);

    if (ImGui::Button(transfer_text, ImVec2(trans_text_size.x + button_size, trans_text_size.y + button_size)))
    {
        appobj::glengine.copyTextureToFBO(tex, width, height);
        // Delete the image to stop hogging memory
        appobj::glengine.deleteTexture(tex);
        show = false;
    };
    ImGui::End();
}
