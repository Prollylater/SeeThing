#include "Shell.h"

#include "AssetPath.h"
#include "IconsCodicons.h"
#include "app/App.h"
#include "app/Platform.h"
#include "doc/Edits.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <filesystem>

namespace ui
{
    namespace
    {
        const std::vector<std::string> imageTypes = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".PNG", ".JPG", ".JPEG"};

        void setupFonts()
        {
            ImGuiIO &io = ImGui::GetIO();
            // Explicit size: merging a sized font into an implicitly sized one asserts
            ImFontConfig base;
            base.SizePixels = 13.0f;
            base.PixelSnapH = true;
            io.Fonts->AddFontDefaultBitmap(&base);
            ImFontConfig icons;
            icons.MergeMode = true;
            icons.PixelSnapH = true;
            icons.GlyphMinAdvanceX = 14.0f;
            io.Fonts->AddFontFromFileTTF(assetPath("fonts/codicon.ttf").c_str(), 16.0f, &icons);
        }

        std::filesystem::path folderOf(const std::string &path)
        {
            std::filesystem::path dir = std::filesystem::path(path).parent_path();
            return dir.empty() ? std::filesystem::current_path() : dir;
        }
    }

    Shell::Shell(App &app, Platform &platform)
        : m_app(app),
          m_openDialog(ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_ConfirmOnEnter),
          m_saveDialog(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir |
                       ImGuiFileBrowserFlags_CloseOnEsc | ImGuiFileBrowserFlags_ConfirmOnEnter)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        // Floating panels are OS windows: square and opaque like the main one
        ImGuiStyle &style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        ImGui_ImplGlfw_InitForOpenGL(platform.window(), true);
        ImGui_ImplOpenGL3_Init("#version 450 core");
        setupFonts();

        m_openDialog.SetTitle("Open image");
        m_openDialog.SetTypeFilters(imageTypes);
        m_saveDialog.SetTypeFilters(imageTypes);
    }

    Shell::~Shell()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Shell::beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Shell::draw()
    {
        drawMenuBar();
        handleShortcuts();

        const ImGuiID dockspace = ImHashStr("SeeThingDockSpace");
        if (m_resetLayout || !ImGui::DockBuilderGetNode(dockspace))
        {
            buildDefaultLayout(dockspace);
            m_resetLayout = false;
        }
        ImGui::DockSpaceOverViewport(dockspace, ImGui::GetMainViewport());

        m_canvas.draw(m_app, *this, m_showImage);
        m_result.draw(m_app, *this);
        m_segmentation.draw(m_app, m_showSegmentation);
        m_layers.draw(m_app, m_showLayers);
        m_history.draw(m_app, m_showHistory);
        m_logPanel.draw(m_app.log(), m_showLog);
        drawControls();
        drawDialogs();
    }

    void Shell::render(Platform &platform)
    {
        ImGui::Render();
        int width = 0, height = 0;
        platform.framebufferSize(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Platform windows switch the current context; put the main one back
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *current = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(current);
        }
    }

    void Shell::buildDefaultLayout(ImGuiID dockspace)
    {
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace, ImGui::GetMainViewport()->WorkSize);
        ImGuiID center = dockspace;
        const ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.28f, nullptr, &center);
        ImGuiID rightTop = right;
        const ImGuiID rightBottom = ImGui::DockBuilderSplitNode(rightTop, ImGuiDir_Down, 0.45f, nullptr, &rightTop);
        const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.22f, nullptr, &center);
        ImGui::DockBuilderDockWindow(CanvasPanel::name, center);
        ImGui::DockBuilderDockWindow(ResultPanel::name, center);
        ImGui::DockBuilderDockWindow(SegmentationPanel::name, rightTop);
        ImGui::DockBuilderDockWindow(LayersPanel::name, rightBottom);
        ImGui::DockBuilderDockWindow(HistoryPanel::name, rightBottom);
        ImGui::DockBuilderDockWindow(LogPanel::name, bottom);
        ImGui::DockBuilderFinish(dockspace);
    }

    void Shell::drawMenuBar()
    {
        if (!ImGui::BeginMainMenuBar())
            return;
        const bool hasImage = m_app.hasImage();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem(ICON_CI_FOLDER_OPENED " Open...", "Ctrl+O"))
                openImageDialog();
            if (ImGui::MenuItem(ICON_CI_SAVE " Save", "Ctrl+S", false, hasImage))
                save();
            if (!m_app.savePath().empty())
                ImGui::SetItemTooltip("Writes %s", m_app.savePath().c_str());
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", false, hasImage))
                saveImageDialog();
            ImGui::Separator();
            if (ImGui::MenuItem("Close Image", "Ctrl+W", false, hasImage))
                m_app.closeImage();
            if (ImGui::MenuItem("Quit", "Ctrl+Q"))
                m_app.quit();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            const doc::History *h = hasImage ? &m_app.document().history() : nullptr;
            const std::string undo = h && h->canUndo() ? "Undo " + h->step(h->cursor() - 1).label() : "Undo";
            const std::string redo = h && h->canRedo() ? "Redo " + h->step(h->cursor()).label() : "Redo";
            if (ImGui::MenuItem(undo.c_str(), "Ctrl+Z", false, h && h->canUndo()))
                m_app.undo();
            if (ImGui::MenuItem(redo.c_str(), "Ctrl+Shift+Z", false, h && h->canRedo()))
                m_app.redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A", false, hasImage))
                doc::selectAll(m_app.document());
            if (ImGui::MenuItem("Deselect", "Ctrl+D", false, hasImage && m_app.document().hasSelection()))
                doc::selectNone(m_app.document());
            if (ImGui::MenuItem("Invert Selection", "Ctrl+Shift+I", false, hasImage))
                doc::invertSelection(m_app.document());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem(CanvasPanel::name, nullptr, &m_showImage);
            ImGui::MenuItem(SegmentationPanel::name, nullptr, &m_showSegmentation);
            ImGui::MenuItem(LayersPanel::name, nullptr, &m_showLayers);
            ImGui::MenuItem(HistoryPanel::name, nullptr, &m_showHistory);
            ImGui::MenuItem(LogPanel::name, nullptr, &m_showLog);
            ImGui::Separator();
            if (ImGui::MenuItem("Fit", "Ctrl+0", false, hasImage))
                m_canvas.view().request = CanvasView::Request::Fit;
            if (ImGui::MenuItem("Actual Size", "Ctrl+1", false, hasImage))
                m_canvas.view().request = CanvasView::Request::ActualSize;
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout"))
            {
                m_showImage = m_showSegmentation = m_showLog = m_showLayers = m_showHistory = true;
                m_resetLayout = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::MenuItem("Controls", nullptr, &m_showControls);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void Shell::handleShortcuts()
    {
        // The dialogs are modal: their keys are theirs
        if (m_openDialog.IsOpened() || m_saveDialog.IsOpened())
            return;
        const ImGuiInputFlags global = ImGuiInputFlags_RouteGlobal;
        const bool hasImage = m_app.hasImage();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, global))
            openImageDialog();
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, global))
            save();
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, global))
            saveImageDialog();
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_W, global))
            m_app.closeImage();
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Q, global))
            m_app.quit();
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_0, global))
            m_canvas.view().request = CanvasView::Request::Fit;
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_1, global))
            m_canvas.view().request = CanvasView::Request::ActualSize;
        // A focused text field keeps its own undo and select all: these routes rank below it
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, global | ImGuiInputFlags_Repeat))
            m_app.undo();
        if (hasImage && (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, global | ImGuiInputFlags_Repeat) ||
                         ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, global | ImGuiInputFlags_Repeat)))
            m_app.redo();
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_A, global))
            doc::selectAll(m_app.document());
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D, global))
            doc::selectNone(m_app.document());
        if (hasImage && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_I, global))
            doc::invertSelection(m_app.document());
    }

    void Shell::openImageDialog()
    {
        if (!m_app.imagePath().empty())
            m_openDialog.SetPwd(folderOf(m_app.imagePath()));
        m_openDialog.Open();
    }

    void Shell::save()
    {
        if (m_app.savePath().empty())
            saveImageDialog();
        else
            m_app.saveImage(m_app.savePath());
    }

    void Shell::saveImageDialog()
    {
        m_saveTarget = SaveTarget::Image;
        m_saveDialog.SetTitle("Save image as");
        const std::string &from = m_app.savePath().empty() ? m_app.imagePath() : m_app.savePath();
        if (!from.empty())
            m_saveDialog.SetPwd(folderOf(from));
        m_saveDialog.SetInputName(from.empty() ? "image.png" : std::filesystem::path(from).stem().string() + "_edited.png");
        m_saveDialog.Open();
    }

    void Shell::saveResultDialog()
    {
        m_saveTarget = SaveTarget::Result;
        m_saveDialog.SetTitle("Save result as");
        if (!m_app.imagePath().empty())
        {
            m_saveDialog.SetPwd(folderOf(m_app.imagePath()));
            m_saveDialog.SetInputName(std::filesystem::path(m_app.imagePath()).stem().string() + "_segmented.png");
        }
        else
        {
            m_saveDialog.SetInputName("result.png");
        }
        m_saveDialog.Open();
    }

    void Shell::drawDialogs()
    {
        m_openDialog.Display();
        if (m_openDialog.HasSelected())
        {
            m_app.openImage(m_openDialog.GetSelected().string());
            m_openDialog.ClearSelected();
        }
        m_saveDialog.Display();
        if (m_saveDialog.HasSelected())
        {
            std::filesystem::path path = m_saveDialog.GetSelected();
            // The format comes from the extension; none means PNG
            if (path.extension().empty())
                path += ".png";
            if (m_saveTarget == SaveTarget::Image)
                m_app.saveImage(path.string());
            else
                m_app.saveResult(path.string());
            m_saveDialog.ClearSelected();
        }
    }

    void Shell::drawControls()
    {
        if (!m_showControls)
            return;
        ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Controls", &m_showControls, ImGuiWindowFlags_NoDocking))
        {
            const char *const rows[][2] = {
                {"Mouse wheel", "Zoom around the cursor"},
                {"Right or middle drag", "Pan"},
                {"Space + left drag", "Pan"},
                {"Left drag", "Use the active tool"},
                {"H, P, I, G", "Hand, pencil, colour picker, bucket"},
                {"M, O, L, W", "Rectangle, ellipse, lasso, magic wand"},
                {"Shift / Alt / both", "Add to, subtract from, intersect the selection"},
                {"S, Alt+click", "Seeds tool; remove a seed"},
                {"Ctrl+0 / Ctrl+1", "Fit / actual size"},
                {"Ctrl+Z", "Undo"},
                {"Ctrl+Shift+Z, Ctrl+Y", "Redo"},
                {"Ctrl+A, Ctrl+D", "Select all, deselect"},
                {"Ctrl+Shift+I", "Invert the selection"},
                {"Ctrl+O, Ctrl+S", "Open, save"},
                {"Ctrl+Shift+S", "Save as"},
                {"Ctrl+W, Ctrl+Q", "Close image, quit"},
                {"Drop a file", "Open it"},
            };
            if (ImGui::BeginTable("##controls", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
            {
                for (const auto &row : rows)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(row[0]);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(row[1]);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}
