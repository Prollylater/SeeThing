void ShowExampleAppDockSpace()
{

    static bool opt_fullscreen = true;
    static bool p_open = false;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.2f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::Begin("DockSpace Demo", &p_open, window_flags);

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::Separator();
            if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
            }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                p_open = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void ShowImageRenderingArea(bool &show, bool &showimrender)
{
    if (!show)
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoCollapse;
    // Set the size of the window the first time it appears

    if (ImGui::Begin("Edit Mode", &show, window_flags))
    {
        if (ImGui::BeginMenuBar())
        {
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    show = false;
                }
                if (ImGui::MenuItem("Save"))
                {
                    show = false;
                }
                if (ImGui::MenuItem("Save as"))
                {
                    show = false;
                }
                if (ImGui::MenuItem("Property"))
                {
                    show = false;
                }

                if (ImGui::MenuItem("Close", "Ctrl+W"))
                {
                    show = false;
                }
                ImGui::EndMenu();
            }
            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            ImGui::EndMenuBar();
        }

        // Calculation regarding center child window

        ImVec2 parent_size = ImGui::GetWindowSize();
        ImVec2 parent_pos = ImGui::GetWindowPos();

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));        // Transparent background
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent background when hovered
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        int countButton = 5;
        ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - countButton * 20.0f, 1.0f));
        ImGui::SameLine();
        const char *icons[] = {ICON_CI_PENCIL, ICON_CI_PAINTCAN, ICON_CI_SEARCH, ICON_CI_SEARCH, ICON_CI_MAP};

        for (int i = 0; i < countButton; ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
            }
            std::string button_label = "Button " + std::to_string(i + 1);
            if (ImGui::Button(icons[i]))
            {
                switch (i)
                {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    showimrender = !showimrender;
                    break;
                default:
                    break;
                }
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("This is the description for %s", button_label.c_str());
                ImGui::EndTooltip();
            }
        }
        ImGui::PopStyleVar();

        ImGui::PopStyleColor(2);

        ImGui::Separator();

        // Center window
        ImVec2 child_size(parent_size.x / 1.5, parent_size.y / 1.4);

        ImVec2 child_pos = ImVec2(parent_pos.x + (parent_size.x - child_size.x) * 0.5f,
                                  parent_pos.y + (parent_size.y - child_size.y) * 0.5f);
        // TODO Any other way ?
        // Responsivity
        if ((child_pos.y - parent_pos.y) < 75.0f)
        {
            child_pos = ImVec2(child_pos.x, 300.0f);
        }
        if ((child_pos.x - parent_pos.x) < 75.0f)
        {
            child_pos = ImVec2(300.0f, child_pos.y);
        }

        ImGui::SetNextWindowPos(child_pos, ImGuiCond_Always);
        // Define some window flags
        ImGuiWindowFlags child_window_flags = 0;

        ImGui::BeginChild("ImgHolger", child_size, true, child_window_flags);

        if (ImGui::IsWindowHovered())
        {
            ImGui::Text("Hovering over Child Window!");
            ImVec2 widgetPos = ImGui::GetItemRectMin(); // Get position of the current widget
                                                        /* if (ImGui::IsMouseHoveringRect(widgetPos, ImVec2(widgetPos.x + 100, widgetPos.y + 20))) {
                                             }*/
            ImGui::Text("%f %f", widgetPos.x, widgetPos.y);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void ShowRegionGrowingArea(bool &show)
{
    static int mode = 0;
    if (!show)
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoCollapse;
    // Set the size of the window the first time it appears

    if (ImGui::Begin("Segmentation Mode", &show, window_flags))
    {

        // Calculation to center child window

        ImVec2 parent_size = ImGui::GetWindowSize();
        ImVec2 parent_pos = ImGui::GetWindowPos();

        ImVec2 child_pos = ImVec2(parent_size.x, parent_pos.y);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));        // Transparent background
        // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent background when hovered
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        int countButton = 5;
        ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - countButton * 20.0f, 1.0f));
        ImGui::SameLine();
        const char *icons[] = {"Slic", "Seed", ICON_CI_SEARCH, ICON_CI_SEARCH, ICON_CI_SEARCH};

        for (int i = 0; i < countButton; ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
            }
            std::string button_label = "Button " + std::to_string(i + 1);
            if (ImGui::Button(icons[i]))
            {
                switch (i)
                {
                case 0:
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    break;
                default:
                    break;
                }
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("This is the description for %s", button_label.c_str());
                ImGui::EndTooltip();
            }
        }
        ImGui::PopStyleVar();

        // ImGui::PopStyleColor(2);

        ImGui::Separator();
    }
    ImGui::End();

}

